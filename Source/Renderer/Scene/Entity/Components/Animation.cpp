#include "RendererPCH.h"

#include "Animation.h"

using namespace DirectX;

namespace
{
    constexpr float TICKS_PER_SEC = 25.0f;

    std::pair<scene::AnimationFrame, scene::AnimationFrame> GetInterpolatedKeyFrame(const std::vector<scene::AnimationFrame>& keyFrames, float normalizedTime)
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

namespace scene
{
    std::map<BoneId, XMMATRIX> Animation::GetBonesTransforms(float deltaTime)
    {
        CurrentTime += deltaTime;

        std::map<BoneId, XMMATRIX> transforms;

        float normalizedTime = std::fmodf(CurrentTime * TicksPerSecond, Duration);
        float interpolation = std::fmodf(CurrentTime * TicksPerSecond, 1.0f);

        const auto& interpolatedKeyFrames = GetInterpolatedKeyFrame(Frames, normalizedTime);
        const AnimationFrame& firstFrame = interpolatedKeyFrames.first;
        const AnimationFrame& secondFrame = interpolatedKeyFrames.second;

        for (const auto& [boneId, rotationQuat] : firstFrame.Rotations)
        {
            XMVECTOR interpolatedRotation = XMQuaternionSlerp(rotationQuat, secondFrame.Rotations.at(boneId), interpolation);
            XMMATRIX rotation = XMMatrixRotationQuaternion(interpolatedRotation);
            transforms[boneId] = rotation;
        }

        for (const auto& [boneId, locationVec] : firstFrame.Locations)
        {
            XMVECTOR interpolatedLocation = XMVectorLerp(locationVec, secondFrame.Locations.at(boneId), interpolation);
            XMMATRIX location = XMMatrixTranslationFromVector(interpolatedLocation);
            transforms[boneId] *= location;
        }

        return transforms;
    }
} // namespace scene
