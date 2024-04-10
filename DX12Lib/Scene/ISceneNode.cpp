#include "stdafx.h"

#include "ISceneNode.h"

using namespace DirectX;

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
