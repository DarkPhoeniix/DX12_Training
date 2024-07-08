#include "stdafx.h"
#include "PointLight.h"

PointLight::PointLight(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color, float range)
    : _position(position)
    , _color(color)
    , _range(range)
    , _intensity(1.0f)
{
}

void PointLight::SetToShader(Core::GraphicsCommandList& commandList, int index) const
{
}

void PointLight::SetPosition(const DirectX::XMVECTOR& position)
{
    _position = position;
}

const DirectX::XMVECTOR& PointLight::GetPosition() const
{
    return _position;
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
