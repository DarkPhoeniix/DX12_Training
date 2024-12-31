#pragma once

#include "ISceneProcessor.h"

class SetupCachedDataProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache) override;

    void ProcessEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache);

private:
    size_t _lightNum = 0;
};

