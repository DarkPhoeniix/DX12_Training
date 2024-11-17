#pragma once

#include "ISceneProcessor.h"

class SetupCachedDataProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList) override;

    void ProcessEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList);

private:
    size_t _lightNum = 0;
};

