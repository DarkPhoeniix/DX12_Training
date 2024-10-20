#pragma once

#include "ILight.h"
#include "Scene/NodeFactory.h"

namespace SceneLayer
{
    class DirectionalLight : public ILight
    {
    public:
        DirectionalLight();
        DirectionalLight(SceneCache* cache, ISceneNode* parent);
        ~DirectionalLight() = default;

        void SetDirection(const DirectX::XMVECTOR& direction);
        const DirectX::XMVECTOR& GetDirection() const;

        void SetColor(const DirectX::XMVECTOR& color);
        const DirectX::XMVECTOR& GetColor() const;

    private:
        using Base = ILight;
    };
} // namespace SceneLayer
