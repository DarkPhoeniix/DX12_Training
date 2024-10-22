#pragma once

#include "Scene/SceneCache.h"
#include "Scene/Nodes/ISceneNode.h"

namespace Core
{
    class Texture;
} // namespace Core

namespace SceneLayer
{
    class FrustumVolume;
    class Camera;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        void Draw(Core::CommandList& commandList);
        void DrawAABB(Core::CommandList& commandList);

        // TODO: remove func, load camera from the file
        void SetCamera(Camera& camera);

        bool LoadScene(const std::string& filepath, Core::CommandList& commandList);

    private:
        void _UploadTexture(Core::Texture* texture, Core::CommandList& commandList);

        std::string _name;

        std::vector<std::shared_ptr<ISceneNode>> _rootNodes;

        SceneCache _cache;
        std::shared_ptr<Core::Resource> _sceneGPUData;
    };
} // namespace SceneLayer
