#include "stdafx.h"

#include "EmptyObject.h"

#include "Scene/NodeFactory.h"

namespace SceneLayer
{
    Register_Node(EmptyObject);

    EmptyObject::EmptyObject()
        : Base()
    {    }
    
    EmptyObject::EmptyObject(SceneCache* cache, ISceneNode* parent)
        : Base(cache, parent)
    {    }
} // namespace SceneLayer
