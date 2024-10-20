#include "stdafx.h"

#include "PointLight.h"

namespace SceneLayer
{
    Register_Node(PointLight);

    PointLight::PointLight()
        : Base()
    {    }

    PointLight::PointLight(SceneCache* cache, ISceneNode* parent)
        : Base(cache, parent)
    {    }

    void PointLight::SetColor(const DirectX::XMVECTOR& color)
    {
        _color = color;
    }

    const DirectX::XMVECTOR& PointLight::GetColor() const
    {
        return _color;
    }

    void PointLight::SetRange(float range)
    {
        _range = range;
    }

    float PointLight::GetRange() const
    {
        return _range;
    }

    void PointLight::SetIntensity(float intensity)
    {
        _intensity = intensity;
    }

    float PointLight::GetIntensity() const
    {
        return _intensity;
    }
} // namespace SceneLayer
