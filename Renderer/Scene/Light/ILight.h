#pragma once

#include "Scene/Nodes/ISceneNode.h"

namespace Core
{
    class GraphicsCommandList;
} // namespace Core

namespace SceneLayer
{
    class ILight : public ISceneNode
    {
    public:
        ILight(Scene* scene, ISceneNode* parent = nullptr);

        // ISceneNode
        void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const override {};
        void DrawAABB(Core::GraphicsCommandList& commandList) const override {};

    protected:
        DirectX::XMVECTOR _position;
        DirectX::XMVECTOR _direction;
        DirectX::XMVECTOR _color;

        float _intensity;
        float _range;
    };
} // namespace SceneLayer
