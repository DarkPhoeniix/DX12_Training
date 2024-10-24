#include "stdafx.h"

#include "DirectionalLight.h"

#include "DXObjects/GraphicsCommandList.h"
#include "Scene/NodeFactory.h"

namespace SceneLayer
{
    Register_Node(DirectionalLight);

    DirectionalLight::DirectionalLight()
        : Base()
    {    }

    DirectionalLight::DirectionalLight(SceneCache* cache, ISceneNode* parent)
        : Base(cache, parent)
    {    }

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
} // namespace SceneLayer
