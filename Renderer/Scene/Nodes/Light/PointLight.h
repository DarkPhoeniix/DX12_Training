#pragma once

#include "ILight.h"
#include "Scene/NodeFactory.h"

namespace SceneLayer
{
    class PointLight : public ILight
    {
    public:
        PointLight();
        PointLight(SceneCache* cache, ISceneNode* parent = nullptr);
        ~PointLight() = default;

        void SetColor(const DirectX::XMVECTOR& color);
        const DirectX::XMVECTOR& GetColor() const;

        void SetRange(float range);
        float GetRange() const;

        void SetIntensity(float intensity);
        float GetIntensity() const;

    private:
        using Base = ILight;
    };
} // namespace SceneLayer
