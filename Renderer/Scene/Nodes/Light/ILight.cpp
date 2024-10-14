#include "stdafx.h"

#include "ILight.h"

namespace SceneLayer
{
    ILight::ILight()
        : Base()
    {    }

    ILight::ILight(SceneCache* cache, ISceneNode* parent)
        : Base(cache, parent)
        , _direction(DirectX::XMVectorZero())
        , _position(DirectX::XMVectorZero())
        , _color(DirectX::XMVectorZero())
        , _intensity(0.0f)
        , _range(0.0f)
    {   }
} // namespace SceneLayer
