#pragma once

#include "Scene/Nodes/ISceneNode.h"

namespace Core
{
    class CommandList;
} // namespace Core

namespace SceneLayer
{
    class ILight : public ISceneNode
    {
    public:
        ILight();
        ILight(SceneCache* cache, ISceneNode* parent = nullptr);

    protected:
        DirectX::XMVECTOR _direction;
        DirectX::XMVECTOR _position;
        DirectX::XMVECTOR _color;

        float _intensity;
        float _range;

    private:
        using Base = ISceneNode;
    };
} // namespace SceneLayer
