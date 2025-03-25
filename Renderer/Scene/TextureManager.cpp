#include "RendererPCH.h"

#include "TextureManager.h"

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

    D3D12_RESOURCE_DESC GetTextureDescription(const DirectX::TexMetadata& metadata)
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

        return textureDesc;
    }
}

void scene::TextureManager::EnqueueTexture(const std::string& filepath)
{
    _uploadQueue.push_back(filepath);
}

void scene::TextureManager::UploadTextures()
{
    std::uint64_t totalRequiredHeapSize = 0;
    std::uint64_t maxTextureSize = 0;

    for (const std::string& filepath : _uploadQueue)
    {
        std::filesystem::path path(filepath);

        DirectX::TexMetadata metadata = GetTextureMetadata(path);
        D3D12_RESOURCE_DESC description = GetTextureDescription(metadata);

        D3D12_RESOURCE_ALLOCATION_INFO allocInfo = dx12::Device::GetDXDevice()->GetResourceAllocationInfo(0, 1, &description);

        std::uint64_t requiredSize = Math::AlignUp(allocInfo.SizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
        totalRequiredHeapSize += requiredSize;
        maxTextureSize = (maxTextureSize < requiredSize) ? requiredSize : maxTextureSize;
    }
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
