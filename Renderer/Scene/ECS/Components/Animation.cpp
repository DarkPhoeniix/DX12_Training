#include "stdafx.h"

#include "Animation.h"

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

std::map<BoneId, DirectX::XMMATRIX> Animation::GetBonesTransforms(float time) const
{
    std::vector<DirectX::XMMATRIX> transforms;

    float normalizedTime = std::fmodf(time * TicksPerSecond, Duration);
    const AnimationFrame& frame = GetInterpolatedKeyFrame(Frames, normalizedTime).first;
    for (const auto& [boneId, boneTransform] : frame.Transforms)
    {
        transforms.push_back(boneTransform);
    }

    return frame.Transforms;
}
