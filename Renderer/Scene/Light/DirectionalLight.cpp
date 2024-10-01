#include "stdafx.h"

#include "DirectionalLight.h"

#include "DXObjects/GraphicsCommandList.h"

namespace SceneLayer
{
    DirectionalLight::DirectionalLight(Scene* scene, ISceneNode* parent)
        : ILight(scene, parent)
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

    void DirectionalLight::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
    {
    }
} // namespace SceneLayer
