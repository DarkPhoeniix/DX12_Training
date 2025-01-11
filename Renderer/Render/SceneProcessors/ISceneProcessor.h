#pragma once

#include "Render/Frame/CacheGPU.h"

namespace dx12
{
    class CommandList;
}

namespace scene
{
    class Scene;
    class Entity;
}

class ISceneProcessor
{
public:
    ISceneProcessor() = default;

    virtual void Process(scene::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache) = 0;
};
