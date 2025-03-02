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
        dx12::ResourceTable* GetResourceTable();
        CacheGPU* GetCache();

        std::shared_ptr<dx12::Resource> GetResource(ResourceId id);

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResource(std::string name, dx12::ResourceDescription desc);
        ResourceId ReadResource(std::string name);
        ResourceId WriteResource(std::string name);

        std::unordered_map<std::string, ResourceId> _mapNameToId;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resources;

        dx12::ResourceTable* _frameResourceTable;
        CacheGPU* _frameCache;
    };
}
