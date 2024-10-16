#include "stdafx.h"
#include "GBuffer.h"

namespace Core
{
    void GBuffer::Init(const DirectX::XMUINT2& size)
    {
        ResourceDescription textureDesc;
        {
            textureDesc.SetSize(size);
            textureDesc.SetFlags(D3D12_RESOURCE_FLAG_NONE);
            textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
            textureDesc.SetLayout(D3D12_TEXTURE_LAYOUT_UNKNOWN);
            textureDesc.SetMipLevels(1);
        }

        _position.SetResourceDescription(textureDesc);
        _position.CreateCommitedResource();
        _position.SetName("RTV Position");

        textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_SNORM);
        _normalSpecular.SetResourceDescription(textureDesc);
        _normalSpecular.CreateCommitedResource();
        _normalSpecular.SetName("RTV Normal+Specular");

        textureDesc.SetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
        _albedoMetalness.SetResourceDescription(textureDesc);
        _albedoMetalness.CreateCommitedResource();
        _albedoMetalness.SetName("RTV Albedo+Metalness");
    }

    Core::Texture& GBuffer::GetPositionTexture()
    {
        return _position;
    }

    const Core::Texture& GBuffer::GetPositionTexture() const
    {
        return _position;
    }

    Core::Texture& GBuffer::GetNormalTexture()
    {
        return _normalSpecular;
    }

    const Core::Texture& GBuffer::GetNormalTexture() const
    {
        return _normalSpecular;
    }

    Core::Texture& GBuffer::GetAlbedoMetalnessTexture()
    {
        return _albedoMetalness;
    }

    const Core::Texture& GBuffer::GetAlbedoMetalnessTexture() const
    {
        return _albedoMetalness;
    }
} // namespace Core
