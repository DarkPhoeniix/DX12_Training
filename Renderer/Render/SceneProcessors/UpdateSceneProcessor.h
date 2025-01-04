#pragma once

#include "ISceneProcessor.h"

class UpdateSceneProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList, CacheGPU* frameCache) override;

    void UpdateEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList, CacheGPU* frameCache);
};
