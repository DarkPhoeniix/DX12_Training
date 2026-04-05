#pragma once

#include "Interfaces.h"
#include "RenderGraphResourceId.h"

#include "Renderer/Helpers/Profiler.h"

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;

    class RenderContext
    {
    public:
        RenderContext(rhi::Device* device, IDescriptorProvider* descriptorProvider);

        std::shared_ptr<rhi::Buffer> GetBuffer(RGBufferId id) const;
        std::shared_ptr<rhi::Texture> GetTexture(RGTextureId id) const;

        std::uint32_t GetBindlessIndex(RGBufferId id, rhi::ResourceViewType viewType) const;
        std::uint32_t GetBindlessIndex(RGTextureId id, rhi::ResourceViewType viewType) const;

        rhi::CPUDescriptor GetDescriptor(RGBufferId id, rhi::ResourceViewType viewType) const;
        rhi::CPUDescriptor GetDescriptor(RGTextureId id, rhi::ResourceViewType viewType) const;

        void SetGPUProfiler(Profiler* gpuProfiler);
        Profiler* GetGPUProfiler() const;

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        [[nodiscard]] RGBufferId DeclareBuffer(const std::string& name, const rhi::BufferDescription& desc, void* data = nullptr, size_t dataSize = 0);
        [[nodiscard]] RGTextureId DeclareTexture(const std::string& name, const rhi::TextureDescription& desc, void* data = nullptr, size_t dataSize = 0);

        [[nodiscard]] RGBufferReadId ReadBuffer(const std::string& name);
        [[nodiscard]] RGBufferWriteId WriteBuffer(const std::string& name);
        [[nodiscard]] RGBufferUploadId UploadBuffer(const std::string& name);
        [[nodiscard]] RGBufferCopySrcId CopySrcBuffer(const std::string& name);
        [[nodiscard]] RGBufferCopyDstId CopyDstBuffer(const std::string& name);
        [[nodiscard]] RGBufferIndirectArgsId IndirectArgBuffer(const std::string& name);

        [[nodiscard]] RGTextureReadId ReadTexture(const std::string& name);
        [[nodiscard]] RGTextureWriteId WriteTexture(const std::string& name);
        [[nodiscard]] RGTextureCopySrcId CopySrcTexture(const std::string& name);
        [[nodiscard]] RGTextureCopyDstId CopyDstTexture(const std::string& name);
        [[nodiscard]] RGTextureRenderTargetId RenderTarget(const std::string& name);
        [[nodiscard]] RGTextureDepthStencilReadId DepthStencilRead(const std::string& name);
        [[nodiscard]] RGTextureDepthStencilWriteId DepthStencilWrite(const std::string& name);

        [[nodiscard]] RGVirtualResourceReadId ReadVirtualResource(const std::string& name);
        [[nodiscard]] RGVirtualResourceWriteId WriteVirtualResource(const std::string& name);

        void FillBuffer(std::shared_ptr<rhi::Buffer> resource, void* data, size_t dataSize = 0);
        void FillTexture(std::shared_ptr<rhi::Texture> resource, void* data, size_t dataSize = 0);

        std::unordered_map<std::string, RGBufferId> _mapNameToBufferId;
        std::unordered_map<std::string, RGTextureId> _mapNameToTextureId;
        std::unordered_map<RGBufferId, std::shared_ptr<rhi::Buffer>> _mapIdToBuffer;
        std::unordered_map<RGTextureId, std::shared_ptr<rhi::Texture>> _mapIdToTexture;

        IDescriptorProvider* _descriptorProvider;
        rhi::Device* _device;

        Profiler* _gpuProfiler;
    };
} // namespace rg
