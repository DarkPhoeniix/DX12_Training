#pragma once

#include "Scene/ECS/Components.h"
#include "Scene/ECS/Components/Armature.h"

struct AnimationFrame
{
    uint32_t Index;
    std::map<BoneId, DirectX::XMMATRIX> Transforms;
};

struct Animation : public IComponent
{
    Animation()
        : IComponent("Animation")
        , TicksPerSecond(0.0f)
        , Duration(0.0f)
    {   }

    std::map<BoneId, DirectX::XMMATRIX> GetBonesTransforms(float time) const;

    std::string Name;

    std::vector<AnimationFrame> Frames;
    float TicksPerSecond;
    float Duration;
};
