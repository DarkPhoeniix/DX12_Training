#pragma once

#include "Render/Frame/CacheGPU.h"

namespace dx12
{
    class CommandList;
}

namespace SceneLayer
{
    class Scene;
    class Entity;
}

class ISceneProcessor
{
public:
    ISceneProcessor() = default;

    virtual void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache) = 0;
};
