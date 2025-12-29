#pragma once

#include "Renderer/Scene/Entity/Entity.h"

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
        Scene() = default;
        Scene(const Scene& other);
        Scene(Scene&& other) noexcept;
        ~Scene() = default;

        Scene& operator=(const Scene& other);
        Scene& operator=(Scene&& other) noexcept;

        void AddRootNode(std::shared_ptr<Entity> entity);

        std::vector<std::shared_ptr<Entity>>& GetRootNodes();
        const std::vector<std::shared_ptr<Entity>>& GetRootNodes() const;

        std::shared_ptr<Entity> FindNodeByName(const std::string& name) const;
        std::shared_ptr<Entity> FindNodeByComponentName(const std::string& componentName) const;

        std::vector<std::shared_ptr<Entity>> FilterNodesByComponent(const std::string& componentName) const;

        void Clear();

        void SetName(const std::string& name);
        const std::string GetName() const;

    private:
        std::string _name;

        std::vector<std::shared_ptr<Entity>> _rootNodes;
    };
} // namespace scene
