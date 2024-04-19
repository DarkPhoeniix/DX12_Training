#pragma once

#include "ISceneNode.h"
#include "SceneNode.h"

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"

#include <fbxsdk.h>

class FrustumVolume;
class Heap;
class DescriptorHeap;
class Texture;

class Scene
{
public:
    Scene();
    ~Scene();

    void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum);
    void DrawAABB(Core::GraphicsCommandList& commandList);

    bool LoadScene(const std::string& name, Core::GraphicsCommandList& commandList);

    friend class ISceneNode;
    friend class SceneNode;

private:
    void _UploadTexture(Core::Texture* texture, Core::GraphicsCommandList& commandList);

    static FbxManager* _FBXManager;
    FbxScene* _scene;

    std::shared_ptr<ISceneNode> _rootNode;

    Core::Heap _texturesHeap;
    Core::DescriptorHeap _texturesDescHeap;
};

