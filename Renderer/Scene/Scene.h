#pragma once

#include "ISceneNode.h"
#include "SceneNode.h"

#include "DXObjects/Heap.h"
#include "DXObjects/ResourceTable.h"
#include "DXObjects/OcclusionQuery.h"

class FrustumVolume;
class DescriptorHeap;
class Texture;
class Camera;

class Scene
{
public:
    Scene();
    ~Scene();

    void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum);
    void DrawAABB(Core::GraphicsCommandList& commandList);

    void SetCamera(Camera& camera);

    bool LoadScene(const std::string& filepath, Core::GraphicsCommandList& commandList);

    friend class ISceneNode;
    friend class SceneNode;

private:
    void _UploadTexture(Core::Texture* texture, Core::GraphicsCommandList& commandList);

    std::vector<std::shared_ptr<ISceneNode>> _rootNodes;
    std::shared_ptr<Core::ResourceTable> _texturesTable;

    Camera* _camera;
    std::shared_ptr<Core::Resource> _sceneData;

    std::string _name;
};

