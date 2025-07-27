#pragma once

#include "Core/ResourceTable.h"
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
        RenderContext(ResourceTable& resourceTable);

        std::uint32_t GetFrameIndex() const;

        dx12::ResourceTable& GetResourceTable();
        CacheGPU& GetCache();

        void BindBindlessTable(dx12::CommandList& commandList) const;

        std::shared_ptr<dx12::Resource> GetResource(ResourceId id);
        std::shared_ptr<dx12::Resource> GetResourceNew(ResourceId id);

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

        const DescriptorHandle& GetStaticResourceHandle(dx12::RenderTargetView rtv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::DepthStencilView dsv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::ShaderResourceView srv) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::UnorderedAccessView uav) const;
        const DescriptorHandle& GetStaticResourceHandle(dx12::ConstantBufferView cbv) const;

        //DescriptorHandle GetTransientResourceHandle(dx12::RenderTargetView rtv);
        //DescriptorHandle GetTransientResourceHandle(dx12::DepthStencilView dsv);
        //DescriptorHandle GetTransientResourceHandle(dx12::ShaderResourceView srv);
        //DescriptorHandle GetTransientResourceHandle(dx12::UnorderedAccessView uav);
        //DescriptorHandle GetTransientResourceHandle(dx12::ConstantBufferView cbv);

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResource(const std::string& name, dx12::ResourceDescription desc);
        ResourceId ReadResource(const std::string& name);
        ResourceId WriteResource(const std::string& name);

        ResourceId CreateResourceNew(const std::string& name, dx12::ResourceDescription desc);
        ResourceId ReadResourceNew(const std::string& name);
        ResourceId WriteResourceNew(const std::string& name);

        std::unordered_map<std::string, ResourceId> _mapNameToId;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resources;

        std::unordered_map<std::string, ResourceId> _mapNameToIdNew;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resourcesNew;

        std::uint32_t _currentFrameIndex;
        dx12::ResourceTable _resourceTable[dx12::BACK_BUFFER_COUNT];
        CacheGPU _cache[dx12::BACK_BUFFER_COUNT];

        ResourceTable& _resourceTableNew;
    };
} // namespace rg
