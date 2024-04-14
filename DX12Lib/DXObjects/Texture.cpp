#include "stdafx.h"

#include "Texture.h"

#include "DXObjects/Device.h"

#include <filesystem>

using namespace DirectX;

namespace
{
    const std::string DDS_EXTENSION = ".dds";
    const std::string HDR_EXTENSION = ".hdr";
    const std::string TGA_EXTENSION = ".tga";
}

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

void Texture::UploadToGPU(ComPtr<ID3D12GraphicsCommandList2> commandList)
{
    if (!_descritptorHeap)
    {
        Logger::Log(LogType::Error, "Invalid descriptor heap for texture " + _name);
        return;
    }

    CreateCommitedResource(D3D12_RESOURCE_STATE_COPY_DEST);
    SetName(_name);

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
    Helper::throwIfFailed(_DXDevice->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_intermediateResource)));
    _intermediateResource->SetName(L"Texture intermediate buffer");

    UpdateSubresources(commandList.Get(), _resource.Get(), _intermediateResource.Get(), 0, 0, subresources.size(), subresources.data());

    _descritptorHeap->PlaceResource(this);
    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = _descritptorHeap->GetResourceCPUHandle(this);

    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
    SRVDesc.Format = _resourceDesc.GetFormat();
    SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // TODO: Only 2D textures supported
    SRVDesc.Texture2D.MipLevels = 1;
    SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    _DXDevice->CreateShaderResourceView(_resource.Get(), &SRVDesc, CPUHandle);
}

void Texture::SetDescriptorHeap(DescriptorHeap* descriptorHeap)
{
    _descritptorHeap = descriptorHeap;
}

DescriptorHeap* Texture::GetDescriptorHeap() const
{
    return _descritptorHeap;
}

void Texture::SetDXDevice(ComPtr<ID3D12Device2> device)
{
    _DXDevice = device;
}

ComPtr<ID3D12Device2> Texture::GetDXDevice() const
{
    return _DXDevice;
}

std::shared_ptr<Texture> Texture::LoadFromFile(const std::string& filepath)
{
    std::filesystem::path path(filepath);
    if (!std::filesystem::exists(path))
    {
        Logger::Log(LogType::Error, "Texture \"" + filepath + "\" doesn't exist");
    }

    std::wstring wFilepath(filepath.begin(), filepath.end());

    std::shared_ptr<Texture> texture = std::make_shared<Texture>();
    texture->_name = path.filename().string();

    HRESULT hr = S_OK;
    TexMetadata& metadata = texture->_metadata;
    if (path.extension() == DDS_EXTENSION)
    {
        hr = LoadFromDDSFile(wFilepath.c_str(), DDS_FLAGS_FORCE_RGB, &metadata, texture->_scratchImage);
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

    if (hr != S_OK)
    {
        Logger::Log(LogType::Error, "Failed to load \"" + filepath + "\" texture");
        return nullptr;
    }

    D3D12_RESOURCE_DESC textureDesc = {};
    switch (metadata.dimension)
    {
    case TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.depth));
        break;
    default:
        Logger::Log(LogType::Error, "Invalid dimension in \"" + filepath + "\" texture");
        return nullptr;
    }

    texture->_resourceDesc = textureDesc;

    return texture;
}
