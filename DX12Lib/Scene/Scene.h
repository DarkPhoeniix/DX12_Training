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

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum);
    void DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList);

    bool LoadScene(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

    friend class ISceneNode;
    friend class SceneNode;

private:
    void _UploadTexture(Texture* texture, ComPtr<ID3D12GraphicsCommandList> commandList);

    static FbxManager* _FBXManager;
    FbxScene* _scene;

    std::shared_ptr<ISceneNode> _rootNode;

    Heap _texturesHeap;
    DescriptorHeap _texturesDescHeap;
};

