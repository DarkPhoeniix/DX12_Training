#include "stdafx.h"
#include "PointLight.h"

PointLight::PointLight(Scene* scene, ISceneNode* parent)
    : ILight(scene, parent)
{
}

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

void PointLight::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
{

}
