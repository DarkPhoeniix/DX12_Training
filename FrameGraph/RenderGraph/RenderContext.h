#pragma once

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;

    using ResourceId = std::uint64_t;

    class RenderContext
    {
    public:
        std::shared_ptr<dx12::Resource> GetResource(ResourceId id);

    private:
        friend class RenderGraph;
        friend class RenderPassBuilder;

        ResourceId CreateResource(std::string name, dx12::ResourceDescription desc);
        ResourceId ReadResource(std::string name);
        ResourceId WriteResource(std::string name);

        std::unordered_map<std::string, ResourceId> _mapNameToId;
        std::unordered_map<ResourceId, std::shared_ptr<dx12::Resource>> _resources;
    };
}
