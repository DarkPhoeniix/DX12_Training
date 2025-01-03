#include "stdafx.h"

#include "GBuffer.h"

#include "CommandList.h"

namespace Core
{
    void GBuffer::Init(const DirectX::XMUINT2& size)
    {
        _RTVDescriptorsHeap.Reset();
        _DSVDescriptorsHeap.Reset();
        _SRVDescriptorsHeap.Reset();

        // Create descriptor heaps (RTV/DSV/SRV)
        {
            dx12::DescriptorHeapDescription desc;
            desc.SetFlags(D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
            desc.SetNumDescriptors(4);

            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            _RTVDescriptorsHeap.Create(desc);
            _RTVDescriptorsHeap.SetName("G-Buffer RTV descriptor heap");

            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            _DSVDescriptorsHeap.Create(desc);
            _DSVDescriptorsHeap.SetName("G-Buffer DSV descriptor heap");

            desc.SetType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            _SRVDescriptorsHeap.Create(desc);
            _SRVDescriptorsHeap.SetName("G-Buffer SRV descriptor heap");
        }

        // Create textures
        {
            dx12::ResourceDescription textureDesc;
            D3D12_CLEAR_VALUE clearValue;
            {
                textureDesc.SetSize(size);
                textureDesc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
                textureDesc.SetDepthOrArraySize(1);
                textureDesc.SetLayout(D3D12_TEXTURE_LAYOUT_UNKNOWN);
                textureDesc.SetMipLevels(1);
                textureDesc.SetAlignment(D3D12_TILED_RESOURCE_TILE_SIZE_IN_BYTES);
                textureDesc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
                textureDesc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::RenderTarget);

                clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                clearValue.Color[0] = 0.0f;
                clearValue.Color[1] = 0.0f;
                clearValue.Color[2] = 0.0f;
                clearValue.Color[3] = 1.0f;
            }

            // Create AlbedoMetalness texture
            textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            textureDesc.SetClearValue(clearValue);

            _albedoMetalness.CreateCommitedResource(textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET);
            _albedoMetalness.SetName("G-Buffer Albedo+Metalness");

            // Create NormalSpecular texture
            clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

            textureDesc.SetFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
            textureDesc.SetClearValue(clearValue);

            _normalSpecular.CreateCommitedResource(textureDesc, D3D12_RESOURCE_STATE_RENDER_TARGET);
            _normalSpecular.SetName("G-Buffer Normal+Specular");

            // Create DepthStencil texture
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1;
            clearValue.DepthStencil.Stencil = 0;

            textureDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
            textureDesc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            textureDesc.SetClearValue(clearValue);
            textureDesc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::DepthStencil);

            _depthStencil.CreateCommitedResource(textureDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE);
            _depthStencil.SetName("G-Buffer DepthStencil");
        }

        // Create resource views
        {
            dx12::Device::CreateRenderTargetView(_albedoMetalness.GetAsRTV(), _RTVDescriptorsHeap);
            dx12::Device::CreateRenderTargetView(_normalSpecular.GetAsRTV(), _RTVDescriptorsHeap);

            dx12::Device::CreateDepthStencilView(_depthStencil.GetAsDSV(), _DSVDescriptorsHeap);

            dx12::Device::CreateShaderResourceView(_albedoMetalness.GetAsSRV(), _SRVDescriptorsHeap);
            dx12::Device::CreateShaderResourceView(_normalSpecular.GetAsSRV(), _SRVDescriptorsHeap);
        }
    }

    void GBuffer::ClearTextures(dx12::CommandList& commandList)
    {
        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList.ClearRTV(GetAlbedoMetalnessTextureCPUHandle(), clearColor);
        commandList.ClearRTV(GetNormalTextureCPUHandle(), clearColor);
        commandList.ClearDSV(GetDepthTextureCPUHandle());
    }

    dx12::DescriptorHeap& GBuffer::GetDescHeap()
    {
        return _RTVDescriptorsHeap;
    }

    dx12::DescriptorHeap& GBuffer::GetUAVHeap()
    {
        return _SRVDescriptorsHeap;
    }

    dx12::Texture& GBuffer::GetDepthTexture()
    {
        return _depthStencil;
    }

    const dx12::Texture& GBuffer::GetDepthTexture() const
    {
        return _depthStencil;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetDepthTextureCPUHandle()
    {
        return _DSVDescriptorsHeap.GetResourceCPUHandle(&_depthStencil, dx12::ResourceViewType::DSV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetDepthTextureGPUHandle()
    {
        return _DSVDescriptorsHeap.GetResourceGPUHandle(&_depthStencil, dx12::ResourceViewType::DSV);
    }

    dx12::Texture& GBuffer::GetNormalTexture()
    {
        return _normalSpecular;
    }

    const dx12::Texture& GBuffer::GetNormalTexture() const
    {
        return _normalSpecular;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetNormalTextureCPUHandle()
    {
        return _RTVDescriptorsHeap.GetResourceCPUHandle(&_normalSpecular, dx12::ResourceViewType::RTV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetNormalTextureGPUHandle()
    {
        return _RTVDescriptorsHeap.GetResourceGPUHandle(&_normalSpecular, dx12::ResourceViewType::RTV);
    }

    dx12::Texture& GBuffer::GetAlbedoMetalnessTexture()
    {
        return _albedoMetalness;
    }

    const dx12::Texture& GBuffer::GetAlbedoMetalnessTexture() const
    {
        return _albedoMetalness;
    }
    
    D3D12_CPU_DESCRIPTOR_HANDLE GBuffer::GetAlbedoMetalnessTextureCPUHandle()
    {
        return _RTVDescriptorsHeap.GetResourceCPUHandle(&_albedoMetalness, dx12::ResourceViewType::RTV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GBuffer::GetAlbedoMetalnessTextureGPUHandle()
    {
        return _RTVDescriptorsHeap.GetResourceGPUHandle(&_albedoMetalness, dx12::ResourceViewType::RTV);
    }
} // namespace Core
