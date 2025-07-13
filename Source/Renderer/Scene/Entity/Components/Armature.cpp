#include "RendererPCH.h"

#include "Armature.h"

#include "Animation.h"

namespace
{
    void UpdateBone(scene::Bone* bone, const DirectX::XMMATRIX parentTransform = DirectX::XMMatrixIdentity())
    {
        if (!bone->PendingUpdate)
        {
            return;
        }

        bone->GlobalTransform = bone->LocalTransform * parentTransform;
        bone->PendingUpdate = false;

        for (scene::Bone* child : bone->Children)
        {
            UpdateBone(child, bone->GlobalTransform);
        }
    }
} // namespace unnamed

namespace scene
{
    Armature::Armature()
        : IComponent("Armature")
        , _root(nullptr)
    {
    }

    void Armature::Init(const std::vector<Bone>& bones)
    {
        _bones = bones;
        for (Bone& bone : _bones)
        {
            _bonesSorted.push_back(&bone);
            bone.PendingUpdate = true;
        }
        std::sort(_bonesSorted.begin(), _bonesSorted.end(), [](Bone* lhs, Bone* rhs) { return lhs->ID < rhs->ID; });

        for (Bone& bone : _bones)
        {
            if (bone.ParentId == -1)
            {
                ASSERT(!_root, "Armature must only 1 root bone");
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
        ASSERT(boneIt != _bones.end(), std::format("Bone with ID {} not found in armature {}", id, _name));

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

        LOG_ERROR("Bone with name '{}' not found in armature '{}'", name, _name);
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
} // namespace scene
