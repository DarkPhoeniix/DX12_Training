#pragma once

#include "ISceneProcessor.h"

class DrawSceneProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, dx12::CommandList& commandList) override;

    void DrawEntity(SceneLayer::Entity& entity, dx12::CommandList& commandList);
};
