#include "stdafx.h"

#include "DirectionalLight.h"

#include "DXObjects/GraphicsCommandList.h"

DirectionalLight::DirectionalLight(const DirectX::XMVECTOR& direction, const DirectX::XMVECTOR& color)
    : _direction(direction)
    , _color(color)
{
}

void DirectionalLight::SetToShader(Core::GraphicsCommandList& commandList, int index) const
{

}

void DirectionalLight::SetDirection(const DirectX::XMVECTOR& direction)
{
    _direction = direction;
}

const DirectX::XMVECTOR& DirectionalLight::GetDirection() const
{
    return _direction;
}

void DirectionalLight::SetColor(const DirectX::XMVECTOR& color)
{
    _color = color;
}

const DirectX::XMVECTOR& DirectionalLight::GetColor() const
{
    return _color;
}
