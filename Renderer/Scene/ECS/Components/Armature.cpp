#include "stdafx.h"

#include "Armature.h"

#include "Animation.h"

namespace
{
    void UpdateBone(Bone& bone, const DirectX::XMMATRIX parentTransform = DirectX::XMMatrixIdentity())
    {
        if (!bone.PendingUpdate)
        {
            return;
        }

        bone.GlobalTransform = bone.LocalTransform * parentTransform;
        bone.PendingUpdate = false;
    }
} // namespace unnamed

Armature::Armature()
    : IComponent("Armature")
{   }

void Armature::ApplyAnimation(const std::map<BoneId, DirectX::XMMATRIX>& boneTransforms)
{
    for (const auto& [id, transform] : boneTransforms)
    {
        SetBoneLocalTransform(id, transform);
    }
}

void Armature::UpdateGlobalTransformations()
{
    for (Bone& bone : _bones)
    {
        DirectX::XMMATRIX parent = DirectX::XMMatrixIdentity();
        if (bone.ParentId != (BoneId)-1)
        {
            auto boneIt = std::find_if(_bones.begin(), _bones.end(), [&bone, &parent](const Bone& b) { return b.ID == bone.ParentId; });
            if (boneIt == _bones.end()) return;
            parent = boneIt->GlobalTransform;
        }
        UpdateBone(bone, parent);
    }
}

void Armature::SetBoneLocalTransform(BoneId id, const DirectX::XMMATRIX& transform)
{
    auto boneIt = std::find_if(_bones.begin(), _bones.end(), [id](const Bone& bone) { return bone.ID == id; });
    if (ASSERT(boneIt != _bones.end(), std::format("Bone with ID {} not found in armature {}", id, _name)))
    {
        return;
    }

    boneIt->LocalTransform = transform;
    boneIt->PendingUpdate = true;
}

void Armature::AddBone(const Bone& bone)
{
    _bones.push_back(bone);
    _bones.back().PendingUpdate = true;
}

void Armature::SetBones(const std::vector<Bone>& bones)
{
    _bones = bones;
    for (Bone& bone : _bones)
    {
        bone.PendingUpdate = true;
    }
}

std::vector<Bone>& Armature::GetBones()
{
    return _bones;
}

const std::vector<Bone>& Armature::GetBones() const
{
    return _bones;
}

void Armature::SetName(const std::string& name)
{
    _name = name;
}

const std::string& Armature::GetName() const
{
    return _name;
}
