#pragma once

#include "ISceneProcessor.h"

class DrawSceneProcessor : public ISceneProcessor
{
public:
    void Process(SceneLayer::Scene& scene, Core::CommandList& commandList) override;

    void DrawEntity(SceneLayer::Entity& entity, Core::CommandList& commandList);
};
