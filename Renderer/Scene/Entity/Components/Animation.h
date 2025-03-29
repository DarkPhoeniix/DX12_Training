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

    class Animation : public IComponent
    {
    public:
        Animation()
            : IComponent("Animation")
            , Frames{}
            , TicksPerSecond(0.0f)
            , Duration(0.0f)
            , CurrentTime(0.0f)
        {
        }

        std::map<BoneId, DirectX::XMMATRIX> GetBonesTransforms(float deltaTime);

        std::string Name;

        std::vector<AnimationFrame> Frames;
        float TicksPerSecond;
        float Duration;
        float CurrentTime;
    };
} // namespace scene
