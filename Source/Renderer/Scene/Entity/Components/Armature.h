#pragma once

#include "Core/GeometryCacheManager.h"
#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/OBBVolume.h"

#include <map>

namespace scene
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

        scene::OBBVolume OBB;
    };

    class Armature : public IComponent
    {
    public:
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

        GeometryHandle GetBoneBufferHandle() const;
        void SetBoneBufferHandle(GeometryHandle handle);

    private:
        Bone* FindBone(BoneId id);

        std::string _name;

        GeometryHandle _boneBufferHandle;

        Bone* _root;
        std::vector<Bone> _bones;
        std::vector<Bone*> _bonesSorted;
    };
} // namespace scene
