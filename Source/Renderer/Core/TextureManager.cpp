#include "RendererPCH.h"

#include "TextureManager.h"

#include "Utility/Helpers.h"

#include "RHI/CommandList.h"
#include "RHI/ResourceBarrier.h"

#include <directx/d3dx12.h>     // D3D12 extension library

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

    static std::unordered_map<DXGI_FORMAT, rhi::Format> s_DXGIFormatToFormat
    {
        { DXGI_FORMAT_R32G32B32A32_TYPELESS , rhi::Format::R32G32B32A32_TYPELESS},
        { DXGI_FORMAT_R32G32B32A32_FLOAT , rhi::Format::R32G32B32A32_FLOAT},
        { DXGI_FORMAT_R32G32B32A32_UINT , rhi::Format::R32G32B32A32_UINT},
        { DXGI_FORMAT_R32G32B32A32_SINT , rhi::Format::R32G32B32A32_SINT},
        { DXGI_FORMAT_R32G32B32_TYPELESS , rhi::Format::R32G32B32_TYPELESS},
        { DXGI_FORMAT_R32G32B32_FLOAT , rhi::Format::R32G32B32_FLOAT},
        { DXGI_FORMAT_R32G32B32_UINT , rhi::Format::R32G32B32_UINT},
        { DXGI_FORMAT_R32G32B32_SINT , rhi::Format::R32G32B32_SINT},
        { DXGI_FORMAT_R16G16B16A16_TYPELESS , rhi::Format::R16G16B16A16_TYPELESS},
        { DXGI_FORMAT_R16G16B16A16_FLOAT , rhi::Format::R16G16B16A16_FLOAT},
        { DXGI_FORMAT_R16G16B16A16_UNORM , rhi::Format::R16G16B16A16_UNORM},
        { DXGI_FORMAT_R16G16B16A16_UINT , rhi::Format::R16G16B16A16_UINT},
        { DXGI_FORMAT_R16G16B16A16_SNORM , rhi::Format::R16G16B16A16_SNORM},
        { DXGI_FORMAT_R16G16B16A16_SINT , rhi::Format::R16G16B16A16_SINT},
        { DXGI_FORMAT_R32G32_TYPELESS , rhi::Format::R32G32_TYPELESS},
        { DXGI_FORMAT_R32G32_FLOAT , rhi::Format::R32G32_FLOAT},
        { DXGI_FORMAT_R32G32_UINT , rhi::Format::R32G32_UINT},
        { DXGI_FORMAT_R32G32_SINT , rhi::Format::R32G32_SINT},
        { DXGI_FORMAT_R32G8X24_TYPELESS , rhi::Format::R32G8X24_TYPELESS},
        { DXGI_FORMAT_D32_FLOAT_S8X24_UINT , rhi::Format::D32_FLOAT_S8X24_UINT},
        { DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS , rhi::Format::R32_FLOAT_X8X24_TYPELESS},
        { DXGI_FORMAT_X32_TYPELESS_G8X24_UINT , rhi::Format::X32_TYPELESS_G8X24_UINT},
        { DXGI_FORMAT_R10G10B10A2_TYPELESS , rhi::Format::R10G10B10A2_TYPELESS},
        { DXGI_FORMAT_R10G10B10A2_UNORM , rhi::Format::R10G10B10A2_UNORM},
        { DXGI_FORMAT_R10G10B10A2_UINT , rhi::Format::R10G10B10A2_UINT},
        { DXGI_FORMAT_R11G11B10_FLOAT , rhi::Format::R11G11B10_FLOAT},
        { DXGI_FORMAT_R8G8B8A8_TYPELESS , rhi::Format::R8G8B8A8_TYPELESS},
        { DXGI_FORMAT_R8G8B8A8_UNORM , rhi::Format::R8G8B8A8_UNORM},
        { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB , rhi::Format::R8G8B8A8_UNORM_SRGB},
        { DXGI_FORMAT_R8G8B8A8_UINT , rhi::Format::R8G8B8A8_UINT},
        { DXGI_FORMAT_R8G8B8A8_SNORM , rhi::Format::R8G8B8A8_SNORM},
        { DXGI_FORMAT_R8G8B8A8_SINT , rhi::Format::R8G8B8A8_SINT},
        { DXGI_FORMAT_R16G16_TYPELESS , rhi::Format::R16G16_TYPELESS},
        { DXGI_FORMAT_R16G16_FLOAT , rhi::Format::R16G16_FLOAT},
        { DXGI_FORMAT_R16G16_UNORM , rhi::Format::R16G16_UNORM},
        { DXGI_FORMAT_R16G16_UINT , rhi::Format::R16G16_UINT},
        { DXGI_FORMAT_R16G16_SNORM , rhi::Format::R16G16_SNORM},
        { DXGI_FORMAT_R16G16_SINT , rhi::Format::R16G16_SINT},
        { DXGI_FORMAT_R32_TYPELESS , rhi::Format::R32_TYPELESS},
        { DXGI_FORMAT_D32_FLOAT , rhi::Format::D32_FLOAT},
        { DXGI_FORMAT_R32_FLOAT , rhi::Format::R32_FLOAT},
        { DXGI_FORMAT_R32_UINT , rhi::Format::R32_UINT},
        { DXGI_FORMAT_R32_SINT , rhi::Format::R32_SINT},
        { DXGI_FORMAT_R24G8_TYPELESS , rhi::Format::R24G8_TYPELESS},
        { DXGI_FORMAT_D24_UNORM_S8_UINT , rhi::Format::D24_UNORM_S8_UINT},
        { DXGI_FORMAT_R24_UNORM_X8_TYPELESS , rhi::Format::R24_UNORM_X8_TYPELESS},
        { DXGI_FORMAT_X24_TYPELESS_G8_UINT , rhi::Format::X24_TYPELESS_G8_UINT},
        { DXGI_FORMAT_R8G8_TYPELESS , rhi::Format::R8G8_TYPELESS},
        { DXGI_FORMAT_R8G8_UNORM , rhi::Format::R8G8_UNORM},
        { DXGI_FORMAT_R8G8_UINT , rhi::Format::R8G8_UINT},
        { DXGI_FORMAT_R8G8_SNORM , rhi::Format::R8G8_SNORM},
        { DXGI_FORMAT_R8G8_SINT , rhi::Format::R8G8_SINT},
        { DXGI_FORMAT_R16_TYPELESS , rhi::Format::R16_TYPELESS},
        { DXGI_FORMAT_R16_FLOAT , rhi::Format::R16_FLOAT},
        { DXGI_FORMAT_D16_UNORM , rhi::Format::D16_UNORM},
        { DXGI_FORMAT_R16_UNORM , rhi::Format::R16_UNORM},
        { DXGI_FORMAT_R16_UINT , rhi::Format::R16_UINT},
        { DXGI_FORMAT_R16_SNORM , rhi::Format::R16_SNORM},
        { DXGI_FORMAT_R16_SINT , rhi::Format::R16_SINT},
        { DXGI_FORMAT_R8_TYPELESS , rhi::Format::R8_TYPELESS},
        { DXGI_FORMAT_R8_UNORM , rhi::Format::R8_UNORM},
        { DXGI_FORMAT_R8_UINT , rhi::Format::R8_UINT},
        { DXGI_FORMAT_R8_SNORM , rhi::Format::R8_SNORM},
        { DXGI_FORMAT_R8_SINT , rhi::Format::R8_SINT},
        { DXGI_FORMAT_A8_UNORM , rhi::Format::A8_UNORM},
        { DXGI_FORMAT_R1_UNORM , rhi::Format::R1_UNORM},
        { DXGI_FORMAT_R9G9B9E5_SHAREDEXP , rhi::Format::R9G9B9E5_SHAREDEXP},
        { DXGI_FORMAT_R8G8_B8G8_UNORM , rhi::Format::R8G8_B8G8_UNORM},
        { DXGI_FORMAT_G8R8_G8B8_UNORM , rhi::Format::G8R8_G8B8_UNORM},
        { DXGI_FORMAT_BC1_TYPELESS , rhi::Format::BC1_TYPELESS},
        { DXGI_FORMAT_BC1_UNORM , rhi::Format::BC1_UNORM},
        { DXGI_FORMAT_BC1_UNORM_SRGB , rhi::Format::BC1_UNORM_SRGB},
        { DXGI_FORMAT_BC2_TYPELESS , rhi::Format::BC2_TYPELESS},
        { DXGI_FORMAT_BC2_UNORM , rhi::Format::BC2_UNORM},
        { DXGI_FORMAT_BC2_UNORM_SRGB , rhi::Format::BC2_UNORM_SRGB},
        { DXGI_FORMAT_BC3_TYPELESS , rhi::Format::BC3_TYPELESS},
        { DXGI_FORMAT_BC3_UNORM , rhi::Format::BC3_UNORM},
        { DXGI_FORMAT_BC3_UNORM_SRGB , rhi::Format::BC3_UNORM_SRGB},
        { DXGI_FORMAT_BC4_TYPELESS , rhi::Format::BC4_TYPELESS},
        { DXGI_FORMAT_BC4_UNORM , rhi::Format::BC4_UNORM},
        { DXGI_FORMAT_BC4_SNORM , rhi::Format::BC4_SNORM},
        { DXGI_FORMAT_BC5_TYPELESS , rhi::Format::BC5_TYPELESS},
        { DXGI_FORMAT_BC5_UNORM , rhi::Format::BC5_UNORM},
        { DXGI_FORMAT_BC5_SNORM , rhi::Format::BC5_SNORM},
        { DXGI_FORMAT_B5G6R5_UNORM , rhi::Format::B5G6R5_UNORM},
        { DXGI_FORMAT_B5G5R5A1_UNORM , rhi::Format::B5G5R5A1_UNORM},
        { DXGI_FORMAT_B8G8R8A8_UNORM , rhi::Format::B8G8R8A8_UNORM},
        { DXGI_FORMAT_B8G8R8X8_UNORM , rhi::Format::B8G8R8X8_UNORM},
        { DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM , rhi::Format::R10G10B10_XR_BIAS_A2_UNORM},
        { DXGI_FORMAT_B8G8R8A8_TYPELESS , rhi::Format::B8G8R8A8_TYPELESS},
        { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB , rhi::Format::B8G8R8A8_UNORM_SRGB},
        { DXGI_FORMAT_B8G8R8X8_TYPELESS , rhi::Format::B8G8R8X8_TYPELESS},
        { DXGI_FORMAT_B8G8R8X8_UNORM_SRGB , rhi::Format::B8G8R8X8_UNORM_SRGB},
        { DXGI_FORMAT_BC6H_TYPELESS , rhi::Format::BC6H_TYPELESS},
        { DXGI_FORMAT_BC6H_UF16 , rhi::Format::BC6H_UF16},
        { DXGI_FORMAT_BC6H_SF16 , rhi::Format::BC6H_SF16},
        { DXGI_FORMAT_BC7_TYPELESS , rhi::Format::BC7_TYPELESS},
        { DXGI_FORMAT_BC7_UNORM , rhi::Format::BC7_UNORM},
        { DXGI_FORMAT_BC7_UNORM_SRGB , rhi::Format::BC7_UNORM_SRGB}
    };

    rhi::Format GetRHIFormat(DXGI_FORMAT format)
    {
        auto it = s_DXGIFormatToFormat.find(format);

        if (it != s_DXGIFormatToFormat.end())
        {
            return it->second;
        }
        else
        {
            return rhi::Format::UNKNOWN;
        }
    }

    rhi::TextureDescription GetTextureDescription(const DirectX::TexMetadata& metadata)
    {
        rhi::TextureDescription description = {};
        switch (metadata.dimension)
        {
        case DirectX::TEX_DIMENSION_TEXTURE1D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.width),
                .Height = 1,
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.arraySize),
                .MipLevels = static_cast<std::uint16_t>(metadata.mipLevels),
                .Format = GetRHIFormat(metadata.format),
                .Dimension = rhi::TextureDimension::Texture1D
            };
            break;
        case DirectX::TEX_DIMENSION_TEXTURE2D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.width),
                .Height = static_cast<std::uint32_t>(metadata.height),
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.arraySize),
                .MipLevels = static_cast<std::uint16_t>(metadata.mipLevels),
                .Format = GetRHIFormat(metadata.format),
                .Dimension = rhi::TextureDimension::Texture2D
            };
            break;
        case DirectX::TEX_DIMENSION_TEXTURE3D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.width),
                .Height = static_cast<std::uint32_t>(metadata.height),
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.arraySize),
                .MipLevels = static_cast<std::uint16_t>(metadata.mipLevels),
                .Format = GetRHIFormat(metadata.format),
                .Dimension = rhi::TextureDimension::Texture3D
            };
            break;
        }

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

    void UploadTextureData(rhi::CommandList* commandList, const std::filesystem::path& path, std::shared_ptr<rhi::Texture> texture, std::shared_ptr<rhi::Buffer> intermediateBuffer)
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

        ID3D12GraphicsCommandList* d3d12CommandList = static_cast<ID3D12GraphicsCommandList*>(commandList->GetNative());
        ID3D12Resource* d3d12TargetResource = static_cast<ID3D12Resource*>(texture->GetNative());
        ID3D12Resource* d3d12IntermediateResource = static_cast<ID3D12Resource*>(intermediateBuffer->GetNative());

        UpdateSubresources(d3d12CommandList,
            d3d12TargetResource,
            d3d12IntermediateResource,
            0, 0, static_cast<std::uint32_t>(subresources.size()),
            subresources.data());

        rhi::TextureBarrier barrier = { texture, rhi::ResourceState::Common, rhi::ResourceState::Common };
        commandList->TransitionBarriers({ barrier });
	}
} // namespace unnamed

std::unique_ptr<TextureManager> TextureManager::_instance = nullptr;

TextureManager::TextureManager(rhi::Device* device)
    : _nextTextureHandle(0)
    , _device(device)
{
}

void TextureManager::Create(rhi::Device* device)
{
    if (!_instance)
    {
        _instance = std::unique_ptr<TextureManager>(new TextureManager(device));
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

    DirectX::TexMetadata metadata = GetTextureMetadata(path);
    rhi::TextureDescription description = GetTextureDescription(metadata);

    UploadInfo info =
    {
        .Name = path.filename().string(),
        .Description = description
    };

    {
        std::lock_guard lock(_queueMutex);

        info.Handle = _nextTextureHandle++;
        auto it = _uploadQueue.try_emplace(filepath, info);
    }

    return handle;
}

void TextureManager::UploadTextures(rhi::CommandList* commandList)
{
    std::uint64_t totalRequiredHeapSize = 0;
    std::uint64_t maxTextureSize = 0;

    std::unordered_map<std::string, UploadInfo> uploadQueueCopy;
    {
        std::lock_guard lock(_queueMutex);

        uploadQueueCopy.swap(_uploadQueue);
    }

    if (uploadQueueCopy.empty())
    {
        return;
    }

    for (const auto& [filepath, uploadInfo] : uploadQueueCopy)
    {
        std::filesystem::path path(filepath);
        std::string textureName = path.filename().string();

        std::shared_lock textureLock(_textureMutex);
        rhi::TextureDescription description = uploadInfo.Description;
        textureLock.unlock();

        rhi::AllocationInfo allocationInfo = _device->GetAllocationInfo(description);
        std::uint32_t requiredSize = Math::AlignUp(allocationInfo.SizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

        rhi::BufferDescription intermediateDesc =
        {
            .Size = requiredSize,
            .Usage = rhi::ResourceUsage::Upload
        };
        _intermediateResources[uploadInfo.Handle] = _device->CreateBuffer(intermediateDesc, rhi::ResourceState::Common, "texture_intermediate_buffer");

        totalRequiredHeapSize += requiredSize;
        maxTextureSize = (maxTextureSize < requiredSize) ? requiredSize : maxTextureSize;
    }

    rhi::HeapDescription heapDesc = { .SizeInBytes = totalRequiredHeapSize };
    _texturesHeap = _device->CreateHeap(heapDesc, "textures_heap");

    for (const auto& [filepath, uploadInfo] : uploadQueueCopy)
    {
        std::filesystem::path path(filepath);
        const std::string textureName = path.filename().string();

        std::shared_lock textureLock(_textureMutex);
        _handleToTexture[uploadInfo.Handle] = _texturesHeap->PlaceResource(uploadInfo.Description, rhi::ResourceState::CopyDest);
        textureLock.unlock();

        UploadTextureData(commandList, path, _handleToTexture[uploadInfo.Handle], _intermediateResources[uploadInfo.Handle]);
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

    if (_texturesHeap)
    {
        _texturesHeap->Reset();
    }

    _nextTextureHandle = 0;
    _handleToTexture.clear();
    _uploadQueue.clear();
    _intermediateResources.clear();
}

TextureHandle TextureManager::AddTexture(std::shared_ptr<rhi::Texture> texture)
{
    std::lock_guard writeLock(_textureMutex);

    TextureHandle handle = _nextTextureHandle++;
    _handleToTexture[handle] = texture;

    return handle;
}

std::shared_ptr<rhi::Texture> TextureManager::GetTexture(TextureHandle handle) const
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
