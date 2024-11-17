#pragma once

#include "Heap.h"
#include "Texture.h"

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

        void ClearTextures(dx12::CommandList& commandList);

        dx12::DescriptorHeap& GetDescHeap();
        dx12::DescriptorHeap& GetUAVHeap();

        dx12::Texture& GetPositionTexture();
        const dx12::Texture& GetPositionTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetPositionTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetPositionTextureGPUHandle();

        dx12::Texture& GetNormalTexture();
        const dx12::Texture& GetNormalTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetNormalTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetNormalTextureGPUHandle();

        dx12::Texture& GetAlbedoMetalnessTexture();
        const dx12::Texture& GetAlbedoMetalnessTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureGPUHandle();

    private:
        dx12::Texture _position;
        dx12::Texture _normalSpecular;
        dx12::Texture _albedoMetalness;

        dx12::DescriptorHeap _descriptorsHeap;
        dx12::DescriptorHeap _UAVHeap;
        dx12::Heap _heap;
    };
} // namespace Core
