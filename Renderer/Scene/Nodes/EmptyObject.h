#pragma once

#include "Scene/Nodes/ISceneNode.h"

namespace SceneLayer
{
    class EmptyObject : public ISceneNode
    {
    public:
        // ISceneNode
        void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const override;
        void DrawAABB(Core::GraphicsCommandList& commandList) const override;

        void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;

    private:
        using Base = ISceneNode;
    };
} // namespace SceneLayer
