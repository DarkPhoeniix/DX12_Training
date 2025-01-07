#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/OBBVolume.h"

namespace SceneLayer
{
    // TODO: change BoneId type
    using BoneId = std::uint32_t;

    struct Bone
    {
        BoneId ID;
        std::string Name;
        BoneId ParentId;
        std::vector<Bone*> Children;

        DirectX::XMMATRIX Offset;
        DirectX::XMMATRIX LocalTransform;
        DirectX::XMMATRIX GlobalTransform;

        bool PendingUpdate;

        SceneLayer::OBBVolume AABB;
    };

    struct Armature : public IComponent
    {
        Armature();

        void Init(const std::vector<Bone>& bones);

        void ApplyAnimation(const std::map<BoneId, DirectX::XMMATRIX>& boneTransforms);
        void UpdateGlobalTransformations();

        void SetBoneLocalTransform(BoneId id, const DirectX::XMMATRIX& transform);

        void AddBone(const Bone& bone);
        std::vector<Bone>& GetBones();
        const std::vector<Bone>& GetBones() const;
        const std::vector<Bone*>& GetSortedBones() const;

        Bone* GetBoneByName(const std::string& name);

        void SetName(const std::string& name);
        const std::string& GetName() const;

        dx12::Resource BoneTransforms;
        dx12::Resource BoneDebugTransforms;

    private:
        Bone* FindBone(BoneId id);

        std::string _name;

        Bone* _root;
        std::vector<Bone> _bones;
        std::vector<Bone*> _bonesSorted;
    };
} // namespace SceneLayer
