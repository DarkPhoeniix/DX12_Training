#pragma once

#include "Resource.h"

class ResourceFactory
{
public:
    template <typename... Args>
    static std::shared_ptr<dx12::Resource> Create(const std::string& name, Args... args)
    {
        // TODO: std::shared_ptr is used for the future abstraction layer over gfx API.
        std::shared_ptr<dx12::Resource> resource = std::shared_ptr<dx12::Resource>(new dx12::Resource(name, std::forward<Args...>(args...)));
        resource->_ID = _nextID++;

        return resource;
    }

private:
    static dx12::ResourceID _nextID;
};
