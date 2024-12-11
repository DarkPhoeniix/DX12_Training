#include "stdafx.h"

#include "Armature.h"

#include "Animation.h"

#include <queue>

namespace
{
    void UpdateBone(Bone* bone, const DirectX::XMMATRIX parentTransform = DirectX::XMMatrixIdentity())
    {
        if (!bone->PendingUpdate)
        {
            return;
        }

        bone->GlobalTransform = bone->LocalTransform * parentTransform;
        bone->PendingUpdate = false;

        for (Bone* child : bone->Children)
        {
            UpdateBone(child, bone->GlobalTransform);
        }
    }
} // namespace unnamed

Armature::Armature()
    : IComponent("Armature")
    , _root(nullptr)
{   }

void Armature::Init(const std::vector<Bone>& bones)
{
    _bones = bones;
    for (Bone& bone : _bones)
    {
        _bonesSorted.push_back(&bone);
        bone.PendingUpdate = true;
    }
    std::sort(_bonesSorted.begin(), _bonesSorted.end(),
        [](Bone* lhs, Bone* rhs) { return lhs->ID < rhs->ID; });

    for (Bone& bone : _bones)
    {
        if (bone.ParentId == -1)
        {
            if (ASSERT(!_root, "Armature must only 1 root bone"))
            {
                return;
            }

            _root = &bone;
        }
        else
        {
            Bone* parent = FindBone(bone.ParentId);
            parent->Children.push_back(&bone);
        }
    }
}

void Armature::ApplyAnimation(const std::map<BoneId, DirectX::XMMATRIX>& boneTransforms)
{
    for (const auto& [id, transform] : boneTransforms)
    {
        SetBoneLocalTransform(id, transform);
    }
}

void Armature::UpdateGlobalTransformations()
{
    UpdateBone(_root);
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

std::vector<Bone>& Armature::GetBones()
{
    return _bones;
}

const std::vector<Bone>& Armature::GetBones() const
{
    return _bones;
}

const std::vector<Bone*>& Armature::GetSortedBones() const
{
    return _bonesSorted;
}

Bone* Armature::GetBoneByName(const std::string& name)
{
    for (Bone* bone : _bonesSorted)
    {
        if (bone->Name == name)
        {
            return bone;
        }
    }

    return nullptr;
}

void Armature::SetName(const std::string& name)
{
    _name = name;
}

const std::string& Armature::GetName() const
{
    return _name;
}

Bone* Armature::FindBone(BoneId id)
{
    for (Bone& bone : _bones)
    {
        if (bone.ID == id)
        {
            return &bone;
        }
    }

    return nullptr;
}
