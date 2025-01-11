#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Entity/Components/Armature.h"

namespace scene
{
    struct AnimationFrame
    {
        uint32_t Index;
        std::map<BoneId, DirectX::XMVECTOR> Locations;
        std::map<BoneId, DirectX::XMVECTOR> Rotations;
    };

    struct Animation : public IComponent
    {
        Animation()
            : IComponent("Animation")
            , TicksPerSecond(0.0f)
            , Duration(0.0f)
        {
        }

        std::map<BoneId, DirectX::XMMATRIX> GetBonesTransforms(float time) const;

        std::string Name;

        std::vector<AnimationFrame> Frames;
        float TicksPerSecond;
        float Duration;
    };
} // namespace scene
