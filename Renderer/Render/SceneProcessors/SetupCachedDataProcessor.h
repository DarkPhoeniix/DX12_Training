#pragma once

#include "ISceneProcessor.h"

class SetupCachedDataProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, Core::CommandList& commandList) override;

    void ProcessEntity(SceneLayer::Entity& entity, Core::CommandList& commandList);

private:
    size_t _lightNum = 0;
};

