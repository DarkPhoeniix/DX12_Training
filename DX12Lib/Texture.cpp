#include "DX12LibPCH.h"

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
    } // namespace unnamed

    void Texture::UploadToGPU(CommandList& commandList)
    {
        ASSERT(_descriptorHeap, "Invalid descriptor heap for texture " + _name);

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
        UINT64 requiredSize = GetRequiredIntermediateSize(_resource.Get(), 0, static_cast<std::uint32_t>(subresources.size()));
        
        D3D12_HEAP_PROPERTIES properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
        helpers::throwIfFailed(dx12::Device::GetDXDevice()->CreateCommittedResource(&properties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&_intermediateResource)));
        _intermediateResource->SetName(L"Texture intermediate buffer");

        UpdateSubresources(commandList.GetDXCommandList().Get(), _resource.Get(), _intermediateResource.Get(), 0, 0, static_cast<std::uint32_t>(subresources.size()), subresources.data());
    }

    void Texture::SetDescriptorHeap(DescriptorHeap* descriptorHeap)
    {
        _descriptorHeap = descriptorHeap;
    }

    DescriptorHeap* Texture::GetDescriptorHeap() const
    {
        return _descriptorHeap;
    }
} // namespace dx12
