#include "stdafx.h"

#include "ISceneNode.h"

using namespace DirectX;

namespace SceneLayer
{
    ISceneNode::ISceneNode()
        : _scene(nullptr)
        , _parent(nullptr)
        , _childNodes{}
        , _transform{}
    {
    }

    ISceneNode::ISceneNode(Scene* scene, ISceneNode* parent)
        : _scene(scene)
        , _parent(parent)
        , _childNodes{}
        , _transform(XMMatrixIdentity())
    {
    }

    ISceneNode::~ISceneNode()
    {
        _scene = nullptr;
        _parent = nullptr;
    }

    void ISceneNode::SetName(const std::string& name)
    {
        _name = name;
    }

    const std::string& ISceneNode::GetName() const
    {
        return _name;
    }

    XMMATRIX ISceneNode::GetGlobalTransform() const
    {
        XMMATRIX globalTransform = _transform;
        if (_parent)
        {
            globalTransform *= _parent->GetGlobalTransform();
        }

        return globalTransform;
    }

    XMMATRIX ISceneNode::GetLocalTransform() const
    {
        return _transform;
    }

    void ISceneNode::SetLocalTransform(const XMMATRIX& transform)
    {
        _transform = transform;
    }

    void ISceneNode::LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList)
    {
        LOG_INFO("Parsing node " + filepath);

        ASSERT(std::filesystem::exists(std::filesystem::path(filepath)), std::format("Failed to parse a node from {}", filepath));
    }
} // namespace SceneLayer
