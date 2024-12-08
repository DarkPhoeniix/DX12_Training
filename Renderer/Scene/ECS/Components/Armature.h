#pragma once

#include "Scene/ECS/Components.h"

// TODO: change BoneId type
using BoneId = std::uint32_t;

struct Bone
{
    BoneId ID;
    std::string Name;
    BoneId ParentId;

    DirectX::XMMATRIX Offset;
    DirectX::XMMATRIX LocalTransform;
    DirectX::XMMATRIX GlobalTransform;

    bool PendingUpdate;
};

struct Armature : public IComponent
{
    Armature();

    void ApplyAnimation(const std::map<BoneId, DirectX::XMMATRIX>& boneTransforms);
    void UpdateGlobalTransformations();

    void SetBoneLocalTransform(BoneId id, const DirectX::XMMATRIX& transform);

    void AddBone(const Bone& bone);
    void SetBones(const std::vector<Bone>& bones);
    std::vector<Bone>& GetBones();
    const std::vector<Bone>& GetBones() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

    dx12::Resource BoneTransforms;

private:
    std::string _name;
    std::vector<Bone> _bones;
};
