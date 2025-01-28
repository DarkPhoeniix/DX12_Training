#pragma once

#include "Scene/SceneCache.h"
#include "Scene/Entity/Entity.h"

namespace dx12
{
    class CommandList;
} // namespace dx12

namespace scene
{
    class Camera;

    class Scene
    {
    public:
        void AddRootNode(std::shared_ptr<Entity> entity);

        std::vector<std::shared_ptr<Entity>>& GetRootNodes();
        std::shared_ptr<Entity> FindNodeByName(const std::string& name) const;
        std::shared_ptr<Entity> FindNodeByComponentName(const std::string& componentName) const;

        std::vector<std::shared_ptr<Entity>> FilterNodesByComponent(const std::string& componentName) const;

        SceneCache& GetCache();

        bool LoadScene(const std::string& filepath, dx12::CommandList& commandList);

    private:
        std::string _name;

        std::vector<std::shared_ptr<Entity>> _rootNodes;

        SceneCache _cache;
    };
} // namespace scene
