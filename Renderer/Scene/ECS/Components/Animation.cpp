#include "stdafx.h"

#include "Animation.h"

using namespace DirectX;

namespace
{
    constexpr float TICKS_PER_SEC = 25.0f;

    std::pair<AnimationFrame, AnimationFrame> GetInterpolatedKeyFrame(const std::vector<AnimationFrame>& keyFrames, float normalizedTime)
    {
        if (normalizedTime < keyFrames.size() - 1)
        {
            size_t startFrameIndex = static_cast<size_t>(normalizedTime);
            size_t endFrameIndex = startFrameIndex + 1;

            return std::make_pair(keyFrames[startFrameIndex], keyFrames[endFrameIndex]);
        }

        return std::make_pair(keyFrames[keyFrames.size() - 1], keyFrames[0]);
    }
}

std::map<BoneId, XMMATRIX> Animation::GetBonesTransforms(float time) const
{
    std::map<BoneId, XMMATRIX> transforms;

    float normalizedTime = std::fmodf(time * TicksPerSecond, Duration);
    const AnimationFrame& frame = GetInterpolatedKeyFrame(Frames, normalizedTime).first;

    for (const auto& [boneId, rotationQuat] : frame.Rotations)
    {
        XMMATRIX rotation;
        rotation = XMMatrixRotationQuaternion(rotationQuat);
        transforms[boneId] = rotation;
    }

    for (const auto& [boneId, locationVec] : frame.Locations)
    {
        XMMATRIX location;
        location = XMMatrixTranslationFromVector(locationVec);
        transforms[boneId] *= location;
    }

    return transforms;
}
