#include "RenderGraphPCH.h"

#include "RenderContext.h"

namespace rg
{
    std::shared_ptr<dx12::Resource> RenderContext::GetResource(ResourceId id)
    {
        auto resourceIt = _resources.find(id);
        if (resourceIt == _resources.end())
        {
            return nullptr;
        }

        return resourceIt->second;
    }

    ResourceId RenderContext::CreateResource(std::string name, dx12::ResourceDescription desc)
    {
        std::shared_ptr<dx12::Resource> resource = std::make_shared<dx12::Resource>();
        resource->SetName(name);
        resource->CreateCommitedResource(desc);

        ResourceId id = _mapNameToId.size();
        _mapNameToId[name] = id;
        _resources[id] = resource;

        return id;
    }

    ResourceId RenderContext::ReadResource(std::string name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (ASSERT(IdIt == _mapNameToId.end(), "Texture is not exist in render graph context"))
        {
            return ResourceId(-1);
        }

        return IdIt->second;
    }

    ResourceId RenderContext::WriteResource(std::string name)
    {
        auto IdIt = _mapNameToId.find(name);
        if (ASSERT(IdIt == _mapNameToId.end(), "Texture is not exist in render graph context"))
        {
            return ResourceId(-1);
        }

        return IdIt->second;
    }
}
