#include "RendererPCH.h"

#include "TextureManager.h"

#include "CommandList.h"
#include "Texture.h"

namespace
{
    constexpr std::uint32_t TEXTURE_TABLE_NUM_DESCRIPTORS = 2048;

    const std::string DDS_EXTENSION = ".dds";
    const std::string HDR_EXTENSION = ".hdr";
    const std::string TGA_EXTENSION = ".tga";

    DirectX::TexMetadata GetTextureMetadata(const std::filesystem::path& path)
    {
        std::filesystem::path extension = path.extension();

        DirectX::TexMetadata metadata;
        if (extension == DDS_EXTENSION)
        {
            DirectX::GetMetadataFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, metadata);
        }
        else if (path.extension() == HDR_EXTENSION)
        {
            DirectX::GetMetadataFromHDRFile(path.c_str(), metadata);
        }
        else if (path.extension() == TGA_EXTENSION)
        {
            DirectX::GetMetadataFromTGAFile(path.c_str(), metadata);
        }
        else
        {
            DirectX::GetMetadataFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, metadata);
        }

        return metadata;
    }

    dx12::ResourceDescription GetTextureDescription(const DirectX::TexMetadata & metadata)
    {
        D3D12_RESOURCE_DESC textureDesc = {};
        switch (metadata.dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE1D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
                metadata.format,
                static_cast<std::uint64_t>(metadata.width),
                static_cast<std::uint16_t>(metadata.arraySize),
                static_cast<std::uint16_t>(metadata.mipLevels));
            break;
        case DirectX::TEX_DIMENSION_TEXTURE2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                metadata.format,
                static_cast<std::uint64_t>(metadata.width),
                static_cast<std::uint32_t>(metadata.height),
                static_cast<std::uint16_t>(metadata.arraySize),
                static_cast<std::uint16_t>(metadata.mipLevels));
            break;
        case DirectX::TEX_DIMENSION_TEXTURE3D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
                metadata.format,
                static_cast<std::uint64_t>(metadata.width),
                static_cast<std::uint32_t>(metadata.height),
                static_cast<std::uint16_t>(metadata.depth),
                static_cast<std::uint16_t>(metadata.mipLevels));
            break;
        }

        dx12::ResourceDescription description(textureDesc);
        description.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Aligned);

        return description;
    }

    DirectX::ScratchImage LoadTextureImage(const std::filesystem::path& path)
    {
        std::filesystem::path extension = path.extension();

        DirectX::ScratchImage image;
        if (extension == DDS_EXTENSION)
        {
            DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
        }
        else if (path.extension() == HDR_EXTENSION)
        {
            DirectX::LoadFromHDRFile(path.c_str(), nullptr, image);
        }
        else if (path.extension() == TGA_EXTENSION)
        {
            DirectX::LoadFromTGAFile(path.c_str(), nullptr, image);
        }
        else
        {
            DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
        }

        return image;
    }

    void UploadTextureData(dx12::CommandList& commandList, const std::filesystem::path& path, std::shared_ptr<dx12::Texture> texture, dx12::Resource& intermediateBuffer)
    {
        DirectX::ScratchImage image = LoadTextureImage(path);

        std::vector<D3D12_SUBRESOURCE_DATA> subresources(image.GetImageCount());
        const DirectX::Image* pImages = image.GetImages();
        for (int i = 0; i < image.GetImageCount(); ++i)
        {
            auto& subresource = subresources[i];
            subresource.RowPitch = pImages[i].rowPitch;
            subresource.SlicePitch = pImages[i].slicePitch;
            subresource.pData = pImages[i].pixels;
        }

        UpdateSubresources(commandList.GetDXCommandList().Get(), 
                           texture->GetDXResource().Get(), 
                           intermediateBuffer.GetDXResource().Get(), 
                           0, 0, static_cast<std::uint32_t>(subresources.size()), 
                           subresources.data());
    }
}


scene::TextureManager::TextureManager()
{
    _texturesTable.Init(TEXTURE_TABLE_NUM_DESCRIPTORS);
}

scene::TextureManager::TextureManager(TextureManager&& manager)
{
    _textures = std::move(manager._textures);
    _uploadQueue = std::move(manager._uploadQueue);
    _texturesTable = std::move(manager._texturesTable);
    _texturesHeap = std::move(manager._texturesHeap);
    _intermediateResources = std::move(manager._intermediateResources);
}

scene::TextureManager& scene::TextureManager::operator=(TextureManager&& manager)
{
    _textures = std::move(manager._textures);
    _uploadQueue = std::move(manager._uploadQueue);
    _texturesTable = std::move(manager._texturesTable);
    _texturesHeap = std::move(manager._texturesHeap);
    _intermediateResources = std::move(manager._intermediateResources);

    return *this;
}

void scene::TextureManager::EnqueueTexture(const std::string& filepath)
{
    _uploadQueue.insert(filepath);
}

void scene::TextureManager::UploadTextures(dx12::CommandList& commandList)
{
    std::uint64_t totalRequiredHeapSize = 0;
    std::uint64_t maxTextureSize = 0;

    for (const std::string& filepath : _uploadQueue)
    {
        std::filesystem::path path(filepath);

        DirectX::TexMetadata metadata = GetTextureMetadata(path);
        dx12::ResourceDescription description = GetTextureDescription(metadata);
        D3D12_RESOURCE_DESC desc = description.CreateDXResourceDescription();

        const std::string textureName = path.filename().string();

        _textures[textureName] = std::make_shared<dx12::Texture>();
        _textures[textureName]->SetResourceDescription(description);
        _textures[textureName]->SetName(textureName);

        D3D12_RESOURCE_ALLOCATION_INFO allocInfo = dx12::Device::GetDXDevice()->GetResourceAllocationInfo(0, 1, &desc);

        std::uint32_t requiredSize = Math::AlignUp(allocInfo.SizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        dx12::ResourceDescription intermediateDesc;
        {
            intermediateDesc.SetSize({ requiredSize, 1 });
            intermediateDesc.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            intermediateDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
        }
        _intermediateResources.emplace(std::make_pair(textureName, intermediateDesc));
        _intermediateResources[textureName].CreateCommitedResource();
        _intermediateResources[textureName].SetName("Texture intermediate buffer");

        totalRequiredHeapSize += requiredSize;
        maxTextureSize = (maxTextureSize < requiredSize) ? requiredSize : maxTextureSize;
    }

    dx12::HeapDescription heapDesc;
    {
        heapDesc.SetSize(totalRequiredHeapSize);
        heapDesc.SetHeapType(D3D12_HEAP_TYPE_DEFAULT);
        heapDesc.SetHeapFlags(D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES);
    }
    _texturesHeap.Create(heapDesc);

    for (const std::string& filepath : _uploadQueue)
    {
        std::filesystem::path path(filepath);
        const std::string textureName = path.filename().string();

        std::shared_ptr<dx12::Texture> texture = _textures[textureName];

        _texturesHeap.PlaceResource(*texture);
        _texturesTable.PlaceResource(texture.get(), dx12::ResourceViewType::SRV);

        UploadTextureData(commandList, path, texture, _intermediateResources[textureName]);
    }
}

void scene::TextureManager::CleanIntermediates()
{
    _intermediateResources.clear();
}

void scene::TextureManager::Clear()
{
    _texturesTable.Reset();
    _texturesHeap.Reset();

    _textures.clear();
    _uploadQueue.clear();
    _intermediateResources.clear();
}

std::shared_ptr<dx12::Texture> scene::TextureManager::GetTexture(const std::string& name) const
{
    auto it = _textures.find(name);
    if (it == _textures.end())
    {
        return nullptr;
    }

    return it->second;
}

dx12::ResourceTable& scene::TextureManager::GetTextureTable()
{
    return _texturesTable;
}
