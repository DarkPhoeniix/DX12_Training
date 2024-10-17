#include "stdafx.h"
#include "GBuffer.h"

namespace Core
{
    void GBuffer::Init(const DirectX::XMUINT2& size)
    {
        Core::DescriptorHeapDescription heapDesc;
        {
            heapDesc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
            heapDesc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            heapDesc.SetNumDescriptors(3);
        }
        _descriptorsHeap.SetDescription(heapDesc);
        _descriptorsHeap.Create();

        D3D12_CLEAR_VALUE clearValueTexTarget;
        ResourceDescription textureDesc;
        {
            textureDesc.SetSize(size);
            textureDesc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
            textureDesc.SetDepthOrArraySize(1);
            textureDesc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
            textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            textureDesc.SetLayout(D3D12_TEXTURE_LAYOUT_UNKNOWN);
            textureDesc.SetMipLevels(1);
            textureDesc.SetAlignment(D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES);


            clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            clearValueTexTarget.Color[0] = 0.0f;
            clearValueTexTarget.Color[1] = 0.0f;
            clearValueTexTarget.Color[2] = 0.0f;
            clearValueTexTarget.Color[3] = 1.0f;

            textureDesc.AddFlags(D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            textureDesc.SetClearValue(clearValueTexTarget);

        }

        _position.SetResourceDescription(textureDesc);
        _position.CreateCommitedResource(D3D12_RESOURCE_STATE_RENDER_TARGET);
        _position.SetName("RTV Position");
        _descriptorsHeap.PlaceResource(&_position);

        textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
        clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        _albedoMetalness.SetResourceDescription(textureDesc);
        _albedoMetalness.CreateCommitedResource(D3D12_RESOURCE_STATE_RENDER_TARGET);
        _albedoMetalness.SetName("RTV Albedo+Metalness");
        _descriptorsHeap.PlaceResource(&_albedoMetalness);

        textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
        clearValueTexTarget.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SetClearValue(clearValueTexTarget);
        _normalSpecular.SetResourceDescription(textureDesc);
        _normalSpecular.CreateCommitedResource(D3D12_RESOURCE_STATE_RENDER_TARGET);
        _normalSpecular.SetName("RTV Normal+Specular");
        _descriptorsHeap.PlaceResource(&_normalSpecular);

        {
            D3D12_RENDER_TARGET_VIEW_DESC renderTargetDesc = {};
            renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            renderTargetDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            renderTargetDesc.Texture2D.MipSlice = 0;
            Core::Device::GetDXDevice()->CreateRenderTargetView(_position.GetDXResource().Get(), &renderTargetDesc, _descriptorsHeap.GetResourceCPUHandle(&_position));
            renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            Core::Device::GetDXDevice()->CreateRenderTargetView(_albedoMetalness.GetDXResource().Get(), &renderTargetDesc, _descriptorsHeap.GetResourceCPUHandle(&_albedoMetalness));
            renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            Core::Device::GetDXDevice()->CreateRenderTargetView(_normalSpecular.GetDXResource().Get(), &renderTargetDesc, _descriptorsHeap.GetResourceCPUHandle(&_normalSpecular));
        }
    }

    Core::Texture& GBuffer::GetPositionTexture()
    {
        return _position;
    }

    const Core::Texture& GBuffer::GetPositionTexture() const
    {
        return _position;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetPositionTextureCPUHandle()
    {
        return _descriptorsHeap.GetResourceCPUHandle(&_position);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetPositionTextureGPUHandle()
    {
        return _descriptorsHeap.GetResourceGPUHandle(&_position);
    }

    Core::Texture& GBuffer::GetNormalTexture()
    {
        return _normalSpecular;
    }

    const Core::Texture& GBuffer::GetNormalTexture() const
    {
        return _normalSpecular;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetNormalTextureCPUHandle()
    {
        return _descriptorsHeap.GetResourceCPUHandle(&_normalSpecular);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetNormalTextureGPUHandle()
    {
        return _descriptorsHeap.GetResourceGPUHandle(&_normalSpecular);
    }

    Core::Texture& GBuffer::GetAlbedoMetalnessTexture()
    {
        return _albedoMetalness;
    }

    const Core::Texture& GBuffer::GetAlbedoMetalnessTexture() const
    {
        return _albedoMetalness;
    }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetAlbedoMetalnessTextureCPUHandle()
    {
        return _descriptorsHeap.GetResourceCPUHandle(&_albedoMetalness);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetAlbedoMetalnessTextureGPUHandle()
    {
        return _descriptorsHeap.GetResourceGPUHandle(&_albedoMetalness);
    }
} // namespace Core