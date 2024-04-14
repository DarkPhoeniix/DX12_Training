#include "stdafx.h"

#include "ISceneNode.h"

using namespace DirectX;

ISceneNode::ISceneNode(const std::string& name, ISceneNode* parent)
    : _name(name)
    , _parent(parent)
    , _transform{}
{
}

ISceneNode::~ISceneNode()
{
    _parent = nullptr;
}

XMMATRIX ISceneNode::GetLocalTransform() const
{
    return _transform;
}

void ISceneNode::SetLocalTransform(const XMMATRIX& transform)
{
    _transform = transform;
}

XMMATRIX ISceneNode::GetGlobalTransform() const
{
    XMMATRIX globalTransform = _transform;
    if (_parent)
        globalTransform *= _parent->GetLocalTransform();

    return globalTransform;
}
