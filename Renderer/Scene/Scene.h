#pragma once

#include "Scene/SceneCache.h"
#include "Scene/Entity.h"

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

        std::vector<std::shared_ptr<Entity>>& GetRootNodes();

        SceneCache& GetCache();

        // TODO: remove func, load camera from the file
        void SetCamera(Camera& camera);

        Core::Resource& GetGPUDesc();

        bool LoadScene(const std::string& filepath, Core::CommandList& commandList);

    private:
        std::string _name;

        std::vector<std::shared_ptr<Entity>> _rootNodes;

        SceneCache _cache;

        Core::Resource _gpuDesc;
    };
} // namespace SceneLayer
