#pragma once

#include "SceneNode.h"

#include <fbxsdk.h>

class Scene
{
public:
    Scene();
    ~Scene();

    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList);

    bool LoadScene(const std::string& name, ComPtr<ID3D12GraphicsCommandList> commandList);

private:
    FbxManager* _FBXManager;
    FbxScene* _scene;

    std::shared_ptr<SceneNode> _rootNode;
};

