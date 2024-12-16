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
    float interpolation = std::fmodf(time * TicksPerSecond, 1.0f);

    const auto& interpolatedKeyFrames = GetInterpolatedKeyFrame(Frames, normalizedTime);
    const AnimationFrame& fisrtFrame = interpolatedKeyFrames.first;
    const AnimationFrame& secondFrame = interpolatedKeyFrames.second;

    for (const auto& [boneId, rotationQuat] : fisrtFrame.Rotations)
    {
        XMVECTOR interpolatedRotation = XMQuaternionSlerp(rotationQuat, secondFrame.Rotations.at(boneId), interpolation);
        XMMATRIX rotation = XMMatrixRotationQuaternion(interpolatedRotation);
        transforms[boneId] = rotation;
    }

    for (const auto& [boneId, locationVec] : fisrtFrame.Locations)
    {
        XMVECTOR interpolatedLocation = XMVectorLerp(locationVec, secondFrame.Locations.at(boneId), interpolation);
        XMMATRIX location = XMMatrixTranslationFromVector(interpolatedLocation);
        transforms[boneId] *= location;
    }

    return transforms;
}
