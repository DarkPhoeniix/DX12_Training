#pragma once

#include "Scene/SceneCache.h"
#include "Scene/ECS/Entity.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

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

        dx12::Resource& GetGPUDesc();

        bool LoadScene(const std::string& filepath, dx12::CommandList& commandList);

    private:
        std::string _name;

        std::vector<std::shared_ptr<Entity>> _rootNodes;

        SceneCache _cache;

        dx12::Resource _gpuDesc;
    };
} // namespace SceneLayer
