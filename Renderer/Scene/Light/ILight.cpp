#include "stdafx.h"

#include "ILight.h"

namespace SceneLayer
{
    ILight::ILight(Scene* scene, ISceneNode* parent)
        : ISceneNode(scene, parent)
    {   }
} // namespace SceneLayer
