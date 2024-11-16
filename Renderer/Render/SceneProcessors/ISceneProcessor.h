#pragma once

namespace Core
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

    virtual void Process(SceneLayer::Scene& scene, Core::CommandList& commandList) = 0;
};
