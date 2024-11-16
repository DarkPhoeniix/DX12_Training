#pragma once

#include "ISceneProcessor.h"

class SetupCachedDataProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, Core::CommandList& commandList) override;
};

