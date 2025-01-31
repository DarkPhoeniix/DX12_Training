#pragma once

#include "ResourceTable.h"
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

        dx12::ResourceTable& GetResourceTable();

        dx12::Texture& GetAlbedoMetalnessTexture();
        const dx12::Texture& GetAlbedoMetalnessTexture() const;

        dx12::Texture& GetNormalTexture();
        const dx12::Texture& GetNormalTexture() const;

        dx12::Texture& GetDepthTexture();
        const dx12::Texture& GetDepthTexture() const;

    private:
        dx12::Texture _albedoMetalness;
        dx12::Texture _normalSpecular;
        dx12::Texture _depthStencil;

        dx12::ResourceTable _resourceTable;
        dx12::Heap _heap;
    };
} // namespace render
