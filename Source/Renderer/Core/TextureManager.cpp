#include "RendererPCH.h"

#include "TextureManager.h"

#include "CommandList.h"

namespace
{
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
        else if (extension == HDR_EXTENSION)
        {
            HRESULT result = DirectX::GetMetadataFromHDRFile(path.c_str(), metadata);
            CHECK(result, "Failed to get metadata from HDR file: " + path.string());
        }
        else if (extension == TGA_EXTENSION)
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

    dx12::ResourceDescription GetTextureDescription(const DirectX::TexMetadata& metadata)
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

		commandList.TransitionBarrier(*texture, dx12::ResourceState::Common);
	}
} // namespace unnamed

std::unique_ptr<TextureManager> TextureManager::_instance = nullptr;

TextureManager::TextureManager()
    : _nextTextureHandle(0)
{
}

void TextureManager::Create()
{
    if (!_instance)
    {
        _instance = std::unique_ptr<TextureManager>(new TextureManager);
    }
    else
    {
        ERROR("TextureManager already created!");
    }
}

void TextureManager::Destroy()
{
    if (_instance)
    {
        _instance.reset();
    }
    else
    {
        ERROR("TextureManager hasn't been created!");
    }
}

TextureManager& TextureManager::Get()
{
    ASSERT(_instance, "TextureManager not created yet!");
    if (_instance)
    {
        return *_instance;
    }
}

TextureHandle TextureManager::EnqueueTexture(const std::string& filepath)
{
    TextureHandle handle = InvalidTextureHandle;

    std::filesystem::path path(filepath);

    {
        std::lock_guard lock(_queueMutex);

        auto it = _uploadQueue.try_emplace(filepath, _nextTextureHandle);
        handle = it.second ? _nextTextureHandle++ : it.first->second;
    }

    DirectX::TexMetadata metadata = GetTextureMetadata(path);
    dx12::ResourceDescription description = GetTextureDescription(metadata);
    D3D12_RESOURCE_DESC desc = description.CreateDXResourceDescription();

    std::string textureName = path.filename().string();

    {
        std::unique_lock textureLock(_textureMutex);
        _handleToTexture[handle] = ResourceFactory::Create(textureName, description);
    }

    return handle;
}

void TextureManager::UploadTextures(dx12::CommandList& commandList)
{
    std::uint64_t totalRequiredHeapSize = 0;
    std::uint64_t maxTextureSize = 0;

    std::unordered_map<std::string, TextureHandle> uploadQueueCopy;
    {
        std::lock_guard lock(_queueMutex);

        uploadQueueCopy.swap(_uploadQueue);
    }

    if (uploadQueueCopy.empty())
    {
        return;
    }

    for (const auto& [filepath, handle] : uploadQueueCopy)
    {
        std::filesystem::path path(filepath);
        std::string textureName = path.filename().string();

        std::shared_lock textureLock(_textureMutex);
        dx12::ResourceDescription description = _handleToTexture[handle]->GetResourceDescription();
        textureLock.unlock();

        D3D12_RESOURCE_DESC desc = description.CreateDXResourceDescription();

        D3D12_RESOURCE_ALLOCATION_INFO allocInfo = dx12::Device::GetDXDevice()->GetResourceAllocationInfo(0, 1, &desc);

        std::uint32_t requiredSize = Math::AlignUp(allocInfo.SizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        dx12::ResourceDescription intermediateDesc;
        {
            intermediateDesc.SetSize({ requiredSize, 1 });
            intermediateDesc.SetLayout(D3D12_TEXTURE_LAYOUT_ROW_MAJOR);
            intermediateDesc.SetResourceType(dx12::ResourceType::Buffer | dx12::ResourceType::Dynamic);
        }
        _intermediateResources[handle] = ResourceFactory::Create("Texture intermediate buffer", intermediateDesc);
        _intermediateResources[handle]->CreateCommitedResource();

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

    for (const auto& [filepath, handle] : uploadQueueCopy)
    {
        std::filesystem::path path(filepath);
        const std::string textureName = path.filename().string();

        std::shared_lock textureLock(_textureMutex);
        std::shared_ptr<dx12::Resource> texture = _handleToTexture[handle];
        textureLock.unlock();

        _texturesHeap.PlaceResource(*texture, dx12::ResourceState::CopyDest);
        UploadTextureData(commandList, path, texture, _intermediateResources[handle]);
    }
}

bool TextureManager::AreTexturesPendingUpload() const
{
    std::lock_guard lock(_queueMutex);

    return !_uploadQueue.empty();
}

void TextureManager::ClearIntermediates()
{
    _intermediateResources.clear();
}

void TextureManager::Clear()
{
    ASSERT(!AreTexturesPendingUpload(), "Cannot clear TextureManager while there are pending texture uploads!");
    std::unique_lock writeLock(_textureMutex);

    _texturesHeap.Reset();

    _nextTextureHandle = 0;
    _handleToTexture.clear();
    _uploadQueue.clear();
    _intermediateResources.clear();
}

TextureHandle TextureManager::AddTexture(std::shared_ptr<dx12::Resource> texture)
{
    std::lock_guard writeLock(_textureMutex);

    TextureHandle handle = _nextTextureHandle++;
    _handleToTexture[handle] = texture;

    return handle;
}

std::shared_ptr<dx12::Resource> TextureManager::GetTexture(TextureHandle handle) const
{
    std::shared_lock readLock(_textureMutex);
    auto it = _handleToTexture.find(handle);
    readLock.unlock();

    if (it == _handleToTexture.end())
    {
        return nullptr;
    }

    return it->second;
}
