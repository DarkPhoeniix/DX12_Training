#include "pch.h"

#include "Texture.h"

#include "CommandList.h"

#include <filesystem>

using namespace DirectX;

namespace dx12
{
    namespace
    {
        const std::string DDS_EXTENSION = ".dds";
        const std::string HDR_EXTENSION = ".hdr";
        const std::string TGA_EXTENSION = ".tga";
        const std::string ERROR_TEXTURE = "Error.dds";
    } // namespace

    Texture::Texture()
        : Resource{}
        , _intermediateResource(nullptr)
        , _scratchImage{}
        , _metadata{}
        , _descritptorHeap(nullptr)
    {
    }

    Texture::~Texture()
    {
        _intermediateResource = nullptr;
        _descritptorHeap = nullptr;
    }

    void Texture::UploadToGPU(CommandList& commandList)
    {
        ASSERT(_descritptorHeap, "Invalid descriptor heap for texture " + _name);

        std::vector<D3D12_SUBRESOURCE_DATA> subresources(_scratchImage.GetImageCount());
        const Image* pImages = _scratchImage.GetImages();
        for (int i = 0; i < _scratchImage.GetImageCount(); ++i)
        {
            auto& subresource = subresources[i];
            subresource.RowPitch = pImages[i].rowPitch;
            subresource.SlicePitch = pImages[i].slicePitch;
            subresource.pData = pImages[i].pixels;
        }

        // Create a temporary (intermediate) resource for uploading the subresources
        UINT64 requiredSize = GetRequiredIntermediateSize(_resource.Get(), 0, subresources.size());

        D3D12_HEAP_PROPERTIES properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
        Helper::throwIfFailed(dx12::Device::GetDXDevice()->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_intermediateResource)));
        _intermediateResource->SetName(L"Texture intermediate buffer");

        UpdateSubresources(commandList.GetDXCommandList().Get(), _resource.Get(), _intermediateResource.Get(), 0, 0, subresources.size(), subresources.data());

        D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = _descritptorHeap->GetResourceCPUHandle(this);

        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = _resourceDesc.GetFormat();
        SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // TODO: Only 2D textures supported
        SRVDesc.Texture2D.MipLevels = _resourceDesc.GetMipLevels();
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        dx12::Device::GetDXDevice()->CreateShaderResourceView(_resource.Get(), &SRVDesc, CPUHandle);
    }

    void Texture::SetDescriptorHeap(DescriptorHeap* descriptorHeap)
    {
        _descritptorHeap = descriptorHeap;
    }

    DescriptorHeap* Texture::GetDescriptorHeap() const
    {
        return _descritptorHeap;
    }

    std::shared_ptr<Texture> Texture::LoadFromFile(std::string filepath)
    {
        std::filesystem::path path(filepath);
        if (ASSERT(std::filesystem::exists(path), "Texture \"" + filepath + "\" doesn't exist. Using " + ERROR_TEXTURE))
        {
            return nullptr;
        }

        std::wstring wFilepath(filepath.begin(), filepath.end());

        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        texture->_name = path.filename().string();

        HRESULT hr = S_OK;
        TexMetadata& metadata = texture->_metadata;
        auto ext = path.extension();
        if (path.extension() == DDS_EXTENSION)
        {
            hr = LoadFromDDSFile(wFilepath.c_str(), DDS_FLAGS_NONE, &metadata, texture->_scratchImage);
        }
        else if (path.extension() == HDR_EXTENSION)
        {
            hr = LoadFromHDRFile(wFilepath.c_str(), &metadata, texture->_scratchImage);
        }
        else if (path.extension() == TGA_EXTENSION)
        {
            hr = LoadFromTGAFile(wFilepath.c_str(), &metadata, texture->_scratchImage);
        }
        else
        {
            hr = LoadFromWICFile(wFilepath.c_str(), WIC_FLAGS_FORCE_RGB, &metadata, texture->_scratchImage);
        }

        if (ASSERT((hr == S_OK), "Failed to load \"" + filepath + "\" texture"))
        {
            return nullptr;
        }

        D3D12_RESOURCE_DESC textureDesc = {};
        switch (metadata.dimension)
        {
        case TEX_DIMENSION_TEXTURE1D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
                metadata.format,
                static_cast<UINT64>(metadata.width),
                static_cast<UINT16>(metadata.arraySize),
                metadata.mipLevels);
            break;
        case TEX_DIMENSION_TEXTURE2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                metadata.format,
                static_cast<UINT64>(metadata.width),
                static_cast<UINT>(metadata.height),
                static_cast<UINT16>(metadata.arraySize),
                metadata.mipLevels);
            break;
        case TEX_DIMENSION_TEXTURE3D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
                metadata.format,
                static_cast<UINT64>(metadata.width),
                static_cast<UINT>(metadata.height),
                static_cast<UINT16>(metadata.depth),
                metadata.mipLevels);
            break;
        default:
            Logger::Log(LogType::Error, "Invalid dimension in \"" + filepath + "\" texture");
            return nullptr;
        }

        texture->_resourceDesc = textureDesc;

        return texture;
    }
} // namespace dx12
