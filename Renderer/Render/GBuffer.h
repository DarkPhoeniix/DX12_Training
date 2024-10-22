#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/Texture.h"

namespace SceneLayer
{
    class Scene;
} // namespace SceneLayer

namespace Core
{
    class GBuffer
    {
    public:
        void Init(const DirectX::XMUINT2& size);

        void ClearTextures(Core::CommandList& commandList);

        Core::DescriptorHeap& GetDescHeap();
        Core::DescriptorHeap& GetUAVHeap();

        Core::Texture& GetPositionTexture();
        const Core::Texture& GetPositionTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetPositionTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetPositionTextureGPUHandle();

        Core::Texture& GetNormalTexture();
        const Core::Texture& GetNormalTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetNormalTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetNormalTextureGPUHandle();

        Core::Texture& GetAlbedoMetalnessTexture();
        const Core::Texture& GetAlbedoMetalnessTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureGPUHandle();

    private:
        Core::Texture _position;
        Core::Texture _normalSpecular;
        Core::Texture _albedoMetalness;

        Core::DescriptorHeap _descriptorsHeap;
        Core::DescriptorHeap _UAVHeap;
        Core::Heap _heap;
    };
} // namespace Core
