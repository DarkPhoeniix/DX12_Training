#include "RendererPCH.h"

#include "TextureManager.h"

#include "Utility/Helpers.h"

#include "RHI/CommandList.h"
#include "RHI/ResourceBarrier.h"

#include "ImageLoader/ImageLoader.h"


namespace
{
    const std::string DDS_EXTENSION = ".dds";
    const std::string HDR_EXTENSION = ".hdr";
    const std::string TGA_EXTENSION = ".tga";

    static std::unordered_map<img::ImageFormat, rhi::Format> s_ImageFormatToRHIFormat
    {
        //{ img::ImageFormat::R32G32B32A32_TYPELESS , rhi::Format::R32G32B32A32_TYPELESS},
        { img::ImageFormat::R32G32B32A32_FLOAT , rhi::Format::R32G32B32A32_FLOAT},
        //{ img::ImageFormat::R32G32B32A32_UINT , rhi::Format::R32G32B32A32_UINT},
        //{ img::ImageFormat::R32G32B32A32_SINT , rhi::Format::R32G32B32A32_SINT},
        //{ img::ImageFormat::R32G32B32_TYPELESS , rhi::Format::R32G32B32_TYPELESS},
        { img::ImageFormat::R32G32B32_FLOAT , rhi::Format::R32G32B32_FLOAT},
        //{ img::ImageFormat::R32G32B32_UINT , rhi::Format::R32G32B32_UINT},
        //{ img::ImageFormat::R32G32B32_SINT , rhi::Format::R32G32B32_SINT},
        //{ img::ImageFormat::R16G16B16A16_TYPELESS , rhi::Format::R16G16B16A16_TYPELESS},
        { img::ImageFormat::R16G16B16A16_FLOAT , rhi::Format::R16G16B16A16_FLOAT},
        { img::ImageFormat::R16G16B16A16_UNORM , rhi::Format::R16G16B16A16_UNORM},
        { img::ImageFormat::R16G16B16A16_UINT , rhi::Format::R16G16B16A16_UINT},
        { img::ImageFormat::R16G16B16A16_SNORM , rhi::Format::R16G16B16A16_SNORM},
        //{ img::ImageFormat::R16G16B16A16_SINT , rhi::Format::R16G16B16A16_SINT},
        //{ img::ImageFormat::R32G32_TYPELESS , rhi::Format::R32G32_TYPELESS},
        { img::ImageFormat::R32G32_FLOAT , rhi::Format::R32G32_FLOAT},
        //{ img::ImageFormat::R32G32_UINT , rhi::Format::R32G32_UINT},
        //{ img::ImageFormat::R32G32_SINT , rhi::Format::R32G32_SINT},
        //{ img::ImageFormat::R32G8X24_TYPELESS , rhi::Format::R32G8X24_TYPELESS},
        //{ img::ImageFormat::D32_FLOAT_S8X24_UINT , rhi::Format::D32_FLOAT_S8X24_UINT},
        //{ img::ImageFormat::R32_FLOAT_X8X24_TYPELESS , rhi::Format::R32_FLOAT_X8X24_TYPELESS},
        //{ img::ImageFormat::X32_TYPELESS_G8X24_UINT , rhi::Format::X32_TYPELESS_G8X24_UINT},
        //{ img::ImageFormat::R10G10B10A2_TYPELESS , rhi::Format::R10G10B10A2_TYPELESS},
        //{ img::ImageFormat::R10G10B10A2_UNORM , rhi::Format::R10G10B10A2_UNORM},
        //{ img::ImageFormat::R10G10B10A2_UINT , rhi::Format::R10G10B10A2_UINT},
        //{ img::ImageFormat::R11G11B10_FLOAT , rhi::Format::R11G11B10_FLOAT},
        //{ img::ImageFormat::R8G8B8A8_TYPELESS , rhi::Format::R8G8B8A8_TYPELESS},
        { img::ImageFormat::R8G8B8A8_UNORM , rhi::Format::R8G8B8A8_UNORM},
        { img::ImageFormat::R8G8B8A8_UNORM_SRGB , rhi::Format::R8G8B8A8_UNORM_SRGB},
        //{ img::ImageFormat::R8G8B8A8_UINT , rhi::Format::R8G8B8A8_UINT},
        { img::ImageFormat::R8G8B8A8_SNORM , rhi::Format::R8G8B8A8_SNORM},
        //{ img::ImageFormat::R8G8B8A8_SINT , rhi::Format::R8G8B8A8_SINT},
        //{ img::ImageFormat::R16G16_TYPELESS , rhi::Format::R16G16_TYPELESS},
        { img::ImageFormat::R16G16_FLOAT , rhi::Format::R16G16_FLOAT},
        { img::ImageFormat::R16G16_UNORM , rhi::Format::R16G16_UNORM},
        { img::ImageFormat::R16G16_SNORM , rhi::Format::R16G16_SNORM},
        //{ img::ImageFormat::R16G16_UINT , rhi::Format::R16G16_UINT},
        //{ img::ImageFormat::R16G16_SINT , rhi::Format::R16G16_SINT},
        //{ img::ImageFormat::R32_TYPELESS , rhi::Format::R32_TYPELESS},
        //{ img::ImageFormat::D32_FLOAT , rhi::Format::D32_FLOAT},
        { img::ImageFormat::R32_FLOAT , rhi::Format::R32_FLOAT},
        //{ img::ImageFormat::R32_UINT , rhi::Format::R32_UINT},
        //{ img::ImageFormat::R32_SINT , rhi::Format::R32_SINT},
        //{ img::ImageFormat::R24G8_TYPELESS , rhi::Format::R24G8_TYPELESS},
        //{ img::ImageFormat::D24_UNORM_S8_UINT , rhi::Format::D24_UNORM_S8_UINT},
        //{ img::ImageFormat::R24_UNORM_X8_TYPELESS , rhi::Format::R24_UNORM_X8_TYPELESS},
        //{ img::ImageFormat::X24_TYPELESS_G8_UINT , rhi::Format::X24_TYPELESS_G8_UINT},
        //{ img::ImageFormat::R8G8_TYPELESS , rhi::Format::R8G8_TYPELESS},
        { img::ImageFormat::R8G8_UNORM , rhi::Format::R8G8_UNORM},
        //{ img::ImageFormat::R8G8_UINT , rhi::Format::R8G8_UINT},
        { img::ImageFormat::R8G8_SNORM , rhi::Format::R8G8_SNORM},
        //{ img::ImageFormat::R8G8_SINT , rhi::Format::R8G8_SINT},
        //{ img::ImageFormat::R16_TYPELESS , rhi::Format::R16_TYPELESS},
        { img::ImageFormat::R16_FLOAT , rhi::Format::R16_FLOAT},
        //{ img::ImageFormat::D16_UNORM , rhi::Format::D16_UNORM},
        { img::ImageFormat::R16_UNORM , rhi::Format::R16_UNORM},
        //{ img::ImageFormat::R16_UINT , rhi::Format::R16_UINT},
        { img::ImageFormat::R16_SNORM , rhi::Format::R16_SNORM},
        //{ img::ImageFormat::R16_SINT , rhi::Format::R16_SINT},
        //{ img::ImageFormat::R8_TYPELESS , rhi::Format::R8_TYPELESS},
        { img::ImageFormat::R8_UNORM , rhi::Format::R8_UNORM},
        //{ img::ImageFormat::R8_UINT , rhi::Format::R8_UINT},
        //{ img::ImageFormat::R8_SNORM , rhi::Format::R8_SNORM},
        //{ img::ImageFormat::R8_SINT , rhi::Format::R8_SINT},
        //{ img::ImageFormat::A8_UNORM , rhi::Format::A8_UNORM},
        //{ img::ImageFormat::R1_UNORM , rhi::Format::R1_UNORM},
        //{ img::ImageFormat::R9G9B9E5_SHAREDEXP , rhi::Format::R9G9B9E5_SHAREDEXP},
        //{ img::ImageFormat::R8G8_B8G8_UNORM , rhi::Format::R8G8_B8G8_UNORM},
        //{ img::ImageFormat::G8R8_G8B8_UNORM , rhi::Format::G8R8_G8B8_UNORM},
        { img::ImageFormat::BC1_TYPELESS , rhi::Format::BC1_TYPELESS},
        { img::ImageFormat::BC1_UNORM , rhi::Format::BC1_UNORM},
        { img::ImageFormat::BC1_UNORM_SRGB , rhi::Format::BC1_UNORM_SRGB},
        { img::ImageFormat::BC2_TYPELESS , rhi::Format::BC2_TYPELESS},
        { img::ImageFormat::BC2_UNORM , rhi::Format::BC2_UNORM},
        { img::ImageFormat::BC2_UNORM_SRGB , rhi::Format::BC2_UNORM_SRGB},
        { img::ImageFormat::BC3_TYPELESS , rhi::Format::BC3_TYPELESS},
        { img::ImageFormat::BC3_UNORM , rhi::Format::BC3_UNORM},
        { img::ImageFormat::BC3_UNORM_SRGB , rhi::Format::BC3_UNORM_SRGB},
        { img::ImageFormat::BC4_TYPELESS , rhi::Format::BC4_TYPELESS},
        { img::ImageFormat::BC4_UNORM , rhi::Format::BC4_UNORM},
        { img::ImageFormat::BC4_SNORM , rhi::Format::BC4_SNORM},
        { img::ImageFormat::BC5_TYPELESS , rhi::Format::BC5_TYPELESS},
        { img::ImageFormat::BC5_UNORM , rhi::Format::BC5_UNORM},
        { img::ImageFormat::BC5_SNORM , rhi::Format::BC5_SNORM},
        //{ img::ImageFormat::B5G6R5_UNORM , rhi::Format::B5G6R5_UNORM},
        //{ img::ImageFormat::B5G5R5A1_UNORM , rhi::Format::B5G5R5A1_UNORM},
        //{ img::ImageFormat::B8G8R8A8_UNORM , rhi::Format::B8G8R8A8_UNORM},
        //{ img::ImageFormat::B8G8R8X8_UNORM , rhi::Format::B8G8R8X8_UNORM},
        //{ img::ImageFormat::R10G10B10_XR_BIAS_A2_UNORM , rhi::Format::R10G10B10_XR_BIAS_A2_UNORM},
        //{ img::ImageFormat::B8G8R8A8_TYPELESS , rhi::Format::B8G8R8A8_TYPELESS},
        //{ img::ImageFormat::B8G8R8A8_UNORM_SRGB , rhi::Format::B8G8R8A8_UNORM_SRGB},
        //{ img::ImageFormat::B8G8R8X8_TYPELESS , rhi::Format::B8G8R8X8_TYPELESS},
        //{ img::ImageFormat::B8G8R8X8_UNORM_SRGB , rhi::Format::B8G8R8X8_UNORM_SRGB},
        { img::ImageFormat::BC6H_TYPELESS , rhi::Format::BC6H_TYPELESS},
        { img::ImageFormat::BC6H_UF16 , rhi::Format::BC6H_UF16},
        { img::ImageFormat::BC6H_SF16 , rhi::Format::BC6H_SF16},
        { img::ImageFormat::BC7_TYPELESS , rhi::Format::BC7_TYPELESS},
        { img::ImageFormat::BC7_UNORM , rhi::Format::BC7_UNORM},
        { img::ImageFormat::BC7_UNORM_SRGB , rhi::Format::BC7_UNORM_SRGB}
    };

    rhi::Format GetRHIFormat(img::ImageFormat format)
    {
        auto it = s_ImageFormatToRHIFormat.find(format);

        if (it != s_ImageFormatToRHIFormat.end())
        {
            return it->second;
        }
        else
        {
            return rhi::Format::UNKNOWN;
        }
    }

    rhi::TextureDescription GetTextureDescription(const img::Metadata& metadata)
    {
        rhi::TextureDescription description = {};

        switch (metadata.Dimension)
        {
        case img::TextureDimension::Texture1D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.Width),
                .Height = 1,
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.ArraySize),
                .MipLevels = static_cast<std::uint16_t>(metadata.MipLevels),
                .Format = GetRHIFormat(metadata.Format),
                .Dimension = rhi::TextureDimension::Texture1D
            };
            break;
        case img::TextureDimension::Texture2D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.Width),
                .Height = static_cast<std::uint32_t>(metadata.Height),
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.ArraySize),
                .MipLevels = static_cast<std::uint16_t>(metadata.MipLevels),
                .Format = GetRHIFormat(metadata.Format),
                .Dimension = rhi::TextureDimension::Texture2D
            };
            break;
        case img::TextureDimension::Texture3D:
            description =
            {
                .Width = static_cast<std::uint32_t>(metadata.Width),
                .Height = static_cast<std::uint32_t>(metadata.Height),
                .DepthOrArraySize = static_cast<std::uint16_t>(metadata.Depth),
                .MipLevels = static_cast<std::uint16_t>(metadata.MipLevels),
                .Format = GetRHIFormat(metadata.Format),
                .Dimension = rhi::TextureDimension::Texture3D
            };
            break;
        }

        return description;
    }

    void UploadTextureData(rhi::CommandList* commandList, const std::filesystem::path& path, std::shared_ptr<rhi::Texture> texture, std::shared_ptr<rhi::Buffer> intermediateBuffer)
    {
        img::Image image = img::LoadImageFromFile(path.string().c_str());

        const std::uint32_t sliceCount = image.GetSliceCount();
        std::vector<rhi::SubresourceData> subresources(sliceCount);
        for (std::uint32_t i = 0; i < sliceCount; ++i)
        {
            const img::ImageSlice& slice = image.GetImageSlice(i);
            subresources[i] =
            {
                .Data       = slice.Pixels,
                .RowPitch   = slice.RowPitch,
                .SlicePitch = slice.SlicePitch
            };
        }

        commandList->CopyBufferToTexture(intermediateBuffer, texture, subresources);

        rhi::TextureBarrier barrier = { texture, rhi::ResourceState::CopyDest, rhi::ResourceState::Common };
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
    std::filesystem::path path(filepath);

    {
        std::lock_guard lock(_queueMutex);

        if (auto it = _uploadQueue.find(filepath); it != _uploadQueue.end())
        {
            return it->second.Handle;
        }
    }

    img::Metadata metadata = img::LoadMetadataFromFile(filepath.c_str());
    rhi::TextureDescription description = GetTextureDescription(metadata);

    UploadInfo info =
    {
        .Handle = InvalidTextureHandle,
        .Name = path.filename().string(),
        .Description = description
    };

    {
        std::lock_guard lock(_queueMutex);

        info.Handle = _nextTextureHandle++;
        auto it = _uploadQueue.try_emplace(filepath, info);
    }

    return info.Handle;
}

void TextureManager::UploadTextures(rhi::CommandList* commandList)
{
    std::unordered_map<std::string, UploadInfo> uploadQueueCopy;
    {
        std::lock_guard lock(_queueMutex);

        uploadQueueCopy.swap(_uploadQueue);
    }

    GPU_SCOPED_EVENT(commandList, "Textures upload", 0);

    if (uploadQueueCopy.empty())
    {
        return;
    }

    for (const auto& [filepath, uploadInfo] : uploadQueueCopy)
    {
        std::filesystem::path path(filepath);
        const std::string textureName = path.filename().string();

        std::shared_lock textureLock(_textureMutex);
        rhi::TextureDescription description = uploadInfo.Description;
        textureLock.unlock();

        rhi::AllocationInfo allocationInfo = _device->GetAllocationInfo(description);
        std::uint32_t requiredSize = Math::AlignUp(allocationInfo.SizeInBytes, allocationInfo.Alignment);

        rhi::BufferDescription intermediateDesc =
        {
            .Size = requiredSize,
            .Usage = rhi::ResourceUsage::Upload
        };
        _intermediateResources[uploadInfo.Handle] = _device->CreateBuffer(intermediateDesc, rhi::ResourceState::Common, "texture_intermediate_buffer");

        std::shared_lock placementLock(_textureMutex);
        _handleToTexture[uploadInfo.Handle] = _device->CreateTexture(description, rhi::ResourceState::CopyDest, textureName);
        placementLock.unlock();

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
