#pragma once

#include "Scene/SceneCache.h"
#include "Scene/ECS/Entity.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

namespace SceneLayer
{
    class Camera;

    class Scene
    {
    public:
        Scene();
        ~Scene();

        std::vector<std::shared_ptr<Entity>>& GetRootNodes();
        std::shared_ptr<Entity> FindNodeByName(const std::string& name) const;
        std::shared_ptr<Entity> FindNodeByComponentName(const std::string& componentName) const;

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
