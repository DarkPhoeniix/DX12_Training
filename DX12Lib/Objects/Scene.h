#pragma once

#include "Interfaces/ISceneNode.h"

#include <fbxsdk.h>

class FrustumVolume;

class Scene
{
public:
    Scene();
    ~Scene();

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum);

    bool LoadScene(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

private:
    static FbxManager* _FBXManager;
    FbxScene* _scene;

    std::shared_ptr<ISceneNode> _rootNode;
};

