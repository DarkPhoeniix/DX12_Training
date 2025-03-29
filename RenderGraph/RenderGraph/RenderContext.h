#pragma once

#include "ResourceTable.h"
#include "Render/Frame/CacheGPU.h"

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;

    using ResourceId = std::uint64_t;

    class RenderContext
    {
    public:
        RenderContext();

        std::uint32_t GetFrameIndex() const;

        dx12::ResourceTable& GetResourceTable();
        CacheGPU& GetCache();

        std::shared_ptr<dx12::Resource> GetResource(ResourceId id);

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(dx12::RenderTargetView rtv);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(dx12::DepthStencilView dsv);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(dx12::ShaderResourceView srv);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(dx12::UnorderedAccessView uav);
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(dx12::ConstantBufferView cbv);

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(dx12::RenderTargetView rtv);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(dx12::DepthStencilView dsv);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(dx12::ShaderResourceView srv);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(dx12::UnorderedAccessView uav);
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(dx12::ConstantBufferView cbv);

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResource(std::string name, dx12::ResourceDescription desc);
        ResourceId ReadResource(std::string name);
        ResourceId WriteResource(std::string name);

        std::unordered_map<std::string, ResourceId> _mapNameToId;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resources;

        std::uint32_t _currentFrameIndex;
        dx12::ResourceTable _resourceTable[dx12::BACK_BUFFER_COUNT];
        CacheGPU _cache[dx12::BACK_BUFFER_COUNT];
    };
} // namespace rg
