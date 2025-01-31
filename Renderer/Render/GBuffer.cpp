#include "RendererPCH.h"

#include "GBuffer.h"

#include "CommandList.h"

namespace render
{
    void GBuffer::Init(const DirectX::XMUINT2& size)
    {
        _resourceTable.Reset();
        _resourceTable.Init(4);

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

            _albedoMetalness.CreateCommitedResource(textureDesc);
            _albedoMetalness.SetName("G-Buffer Albedo+Metalness");

            // Create NormalSpecular texture
            clearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

            textureDesc.SetFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
            textureDesc.SetClearValue(clearValue);

            _normalSpecular.CreateCommitedResource(textureDesc);
            _normalSpecular.SetName("G-Buffer Normal+Specular");

            // Create DepthStencil texture
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1;
            clearValue.DepthStencil.Stencil = 0;

            textureDesc.SetFormat(DXGI_FORMAT_D32_FLOAT);
            textureDesc.SetFlags(D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            textureDesc.SetClearValue(clearValue);
            textureDesc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::DepthStencil);

            _depthStencil.CreateCommitedResource(textureDesc);
            _depthStencil.SetName("G-Buffer DepthStencil");
        }

        // Create resource views
        {
            _resourceTable.PlaceResource(&_albedoMetalness, dx12::ResourceViewType::RTV);
            _resourceTable.PlaceResource(&_normalSpecular, dx12::ResourceViewType::RTV);

            _resourceTable.PlaceResource(&_depthStencil, dx12::ResourceViewType::DSV);

            _resourceTable.PlaceResource(&_albedoMetalness, dx12::ResourceViewType::SRV);
            _resourceTable.PlaceResource(&_normalSpecular, dx12::ResourceViewType::SRV);
            _resourceTable.PlaceResource(&_depthStencil, dx12::ResourceViewType::SRV);
        }
    }

    void GBuffer::ClearTextures(dx12::CommandList& commandList)
    {
        FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        commandList.ClearRTV(_resourceTable.GetResourceCPUHandle(&_albedoMetalness, dx12::ResourceViewType::RTV), clearColor);
        commandList.ClearRTV(_resourceTable.GetResourceCPUHandle(&_normalSpecular, dx12::ResourceViewType::RTV), clearColor);
        commandList.ClearDSV(_resourceTable.GetResourceCPUHandle(&_depthStencil, dx12::ResourceViewType::DSV));
    }

    dx12::ResourceTable& GBuffer::GetResourceTable()
    {
        return _resourceTable;
    }

    dx12::Texture& GBuffer::GetDepthTexture()
    {
        return _depthStencil;
    }

    const dx12::Texture& GBuffer::GetDepthTexture() const
    {
        return _depthStencil;
    }

    dx12::Texture& GBuffer::GetNormalTexture()
    {
        return _normalSpecular;
    }

    const dx12::Texture& GBuffer::GetNormalTexture() const
    {
        return _normalSpecular;
    }

    dx12::Texture& GBuffer::GetAlbedoMetalnessTexture()
    {
        return _albedoMetalness;
    }

    const dx12::Texture& GBuffer::GetAlbedoMetalnessTexture() const
    {
        return _albedoMetalness;
    }
} // namespace render
