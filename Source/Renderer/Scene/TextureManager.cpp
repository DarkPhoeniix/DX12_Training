#include "RendererPCH.h"

#include "TextureManager.h"

#include "CommandList.h"
#include "Texture.h"

namespace
{
    constexpr std::uint32_t TEXTURE_TABLE_NUM_DESCRIPTORS = 2048u;

    const std::string DDS_EXTENSION = ".dds";
    const std::string HDR_EXTENSION = ".hdr";
    const std::string TGA_EXTENSION = ".tga";

    DirectX::TexMetadata GetTextureMetadata(const std::filesystem::path& path)
    {
        std::filesystem::path extension = path.extension();

        DirectX::TexMetadata metadata;
        if (extension == DDS_EXTENSION)
        {
            HRESULT result = DirectX::GetMetadataFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, metadata);
            CHECK(result, "Failed to get metadata from DDS file: " + path.string());
        }
        else if (path.extension() == HDR_EXTENSION)
        {
            HRESULT result = DirectX::GetMetadataFromHDRFile(path.c_str(), metadata);
            CHECK(result, "Failed to get metadata from HDR file: " + path.string());
        }
        else if (path.extension() == TGA_EXTENSION)
        {
            HRESULT result = DirectX::GetMetadataFromTGAFile(path.c_str(), metadata);
            CHECK(result, "Failed to get metadata from TGA file: " + path.string());
        }
        else
        {
            HRESULT result = DirectX::GetMetadataFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, metadata);
            CHECK(result, "Failed to get metadata from WIC file: " + path.string());
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
            HRESULT result = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
            CHECK(result, "Failed to load DDS file: " + path.string());
        }
        else if (path.extension() == HDR_EXTENSION)
        {
            HRESULT result = DirectX::LoadFromHDRFile(path.c_str(), nullptr, image);
            CHECK(result, "Failed to load HDR file: " + path.string());
        }
        else if (path.extension() == TGA_EXTENSION)
        {
            HRESULT result = DirectX::LoadFromTGAFile(path.c_str(), nullptr, image);
            CHECK(result, "Failed to load TGA file: " + path.string());
        }
        else
        {
            HRESULT result = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
            CHECK(result, "Failed to load WIC file: " + path.string());
        }

        return image;
    }

    void UploadTextureData(dx12::CommandList& commandList, const std::filesystem::path& path, std::shared_ptr<dx12::Resource> texture, std::shared_ptr<dx12::Resource> intermediateBuffer)
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
                           intermediateBuffer->GetDXResource().Get(), 
                           0, 0, static_cast<std::uint32_t>(subresources.size()), 
                           subresources.data());

        commandList.TransitionBarrier(*texture, D3D12_RESOURCE_STATE_COMMON);
    }
}

namespace scene
{
    TextureManager::TextureManager()
    {
        _texturesTable.Init(TEXTURE_TABLE_NUM_DESCRIPTORS);
    }

    TextureManager::TextureManager(const TextureManager& other)
        : _textures(other._textures)
        , _uploadQueue(other._uploadQueue)
        , _texturesTable(other._texturesTable)
        , _texturesHeap(other._texturesHeap)
        , _intermediateResources(other._intermediateResources)
    {
    }

    TextureManager::TextureManager(TextureManager&& other) noexcept
        : _textures(std::move(other._textures))
        , _uploadQueue(std::move(other._uploadQueue))
        , _texturesTable(std::move(other._texturesTable))
        , _texturesHeap(std::move(other._texturesHeap))
        , _intermediateResources(std::move(other._intermediateResources))
    {
    }

    TextureManager& TextureManager::operator=(const TextureManager& other)
    {
        if (this != &other)
        {
            _textures = other._textures;
            _uploadQueue = other._uploadQueue;
            _texturesTable = other._texturesTable;
            _texturesHeap = other._texturesHeap;
            _intermediateResources = other._intermediateResources;
        }

        return *this;
    }

    TextureManager& TextureManager::operator=(TextureManager&& other) noexcept
    {
        if (this != &other)
        {
            _textures = std::move(other._textures);
            _uploadQueue = std::move(other._uploadQueue);
            _texturesTable = std::move(other._texturesTable);
            _texturesHeap = std::move(other._texturesHeap);
            _intermediateResources = std::move(other._intermediateResources);
        }

        return *this;
    }

    void TextureManager::EnqueueTexture(const std::string& filepath)
    {
        _uploadQueue.insert(filepath);
    }

    void TextureManager::UploadTextures(dx12::CommandList& commandList)
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

            _textures[textureName] = ResourceFactory::Create(textureName, description);

            D3D12_RESOURCE_ALLOCATION_INFO allocInfo = dx12::Device::GetDXDevice()->GetResourceAllocationInfo(0, 1, &desc);

            std::uint32_t requiredSize = Math::AlignUp(allocInfo.SizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

            dx12::ResourceDescription intermediateDesc;
            {
                intermediateDesc.SetSize({ requiredSize, 1 });
                intermediateDesc.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
                intermediateDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
            }
            std::shared_ptr<dx12::Resource> intermediate = ResourceFactory::Create(textureName, intermediateDesc);
            intermediate->CreateCommitedResource();
            _intermediateResources.emplace(textureName, intermediate);

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

            std::shared_ptr<dx12::Resource> texture = _textures[textureName];

            _texturesHeap.PlaceResource(*texture, D3D12_RESOURCE_STATE_COPY_DEST);
            _texturesTable.PlaceResource(texture, dx12::ResourceViewType::SRV);

            UploadTextureData(commandList, path, texture, _intermediateResources[textureName]);
        }
    }

    void TextureManager::CleanIntermediates()
    {
        _intermediateResources.clear();
    }

    void TextureManager::Clear()
    {
        _texturesTable.Reset();
        _texturesHeap.Reset();

        _textures.clear();
        _uploadQueue.clear();
        _intermediateResources.clear();
    }

    void TextureManager::AddTexture(std::shared_ptr<dx12::Resource> texture, dx12::ResourceViewType viewType)
    {
        auto it = _textures.find(texture->GetName());
        if (it == _textures.end())
        {
            _textures.insert(std::make_pair(texture->GetName(), texture));
        }

        _texturesTable.PlaceResourceIfNotExist(texture, viewType);
    }

    std::shared_ptr<dx12::Resource> TextureManager::GetTexture(const std::string& name) const
    {
        auto it = _textures.find(name);
        if (it == _textures.end())
        {
            return nullptr;
        }

        return it->second;
    }

    dx12::ResourceTable& TextureManager::GetTextureTable()
    {
        return _texturesTable;
    }
} // namespace scene
