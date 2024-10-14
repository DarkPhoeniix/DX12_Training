#pragma once

#include "Scene/Nodes/ISceneNode.h"

namespace SceneLayer
{
    class Scene;

    class EmptyObject : public ISceneNode
    {
    public:
        EmptyObject();
        EmptyObject(SceneCache* cache, ISceneNode* parent = nullptr);

    private:
        using Base = ISceneNode;
    };
} // namespace SceneLayer
