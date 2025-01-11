#pragma once

#include "Heap.h"
#include "Texture.h"

namespace scene
{
    class Scene;
} // namespace scene

namespace render
{
    class GBuffer
    {
    public:
        void Init(const DirectX::XMUINT2& size);

        void ClearTextures(dx12::CommandList& commandList);

        dx12::DescriptorHeap& GetDescHeap();
        dx12::DescriptorHeap& GetUAVHeap();

        dx12::Texture& GetAlbedoMetalnessTexture();
        const dx12::Texture& GetAlbedoMetalnessTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetAlbedoMetalnessTextureGPUHandle();

        dx12::Texture& GetNormalTexture();
        const dx12::Texture& GetNormalTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetNormalTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetNormalTextureGPUHandle();

        dx12::Texture& GetDepthTexture();
        const dx12::Texture& GetDepthTexture() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDepthTextureCPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE GetDepthTextureGPUHandle();

    private:
        dx12::Texture _albedoMetalness;
        dx12::Texture _normalSpecular;
        dx12::Texture _depthStencil;

        dx12::DescriptorHeap _RTVDescriptorsHeap;
        dx12::DescriptorHeap _DSVDescriptorsHeap;
        dx12::DescriptorHeap _SRVDescriptorsHeap;
        dx12::Heap _heap;
    };
} // namespace render
