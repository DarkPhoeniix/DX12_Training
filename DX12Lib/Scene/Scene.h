#pragma once

#include "ISceneNode.h"
#include "SceneNode.h"

#include "DXObjects/Heap.h"
#include "DXObjects/ResourceTable.h"
#include "DXObjects/OcclusionQuery.h"

class FrustumVolume;
class DescriptorHeap;
class Texture;

class Scene
{
public:
    Scene();
    ~Scene();

    void RunOcclusion(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum);
    void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum);
    void DrawAABB(Core::GraphicsCommandList& commandList);

    bool LoadScene(const std::string& filepath, Core::GraphicsCommandList& commandList);

    friend class ISceneNode;
    friend class SceneNode;

private:
    void _UploadTexture(Core::Texture* texture, Core::GraphicsCommandList& commandList);

    static FbxManager* _FBXManager;
    FbxScene* _scene;

    std::vector<std::shared_ptr<ISceneNode>> _rootNodes;

    std::shared_ptr<Core::ResourceTable> _texturesTable;
    Core::OcclusionQuery _occlusionQuery;

    std::string _name;
};

