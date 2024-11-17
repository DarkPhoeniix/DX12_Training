#pragma once

#include "Scene/ECS/Components.h"

namespace SceneLayer
{
    class Entity;
    class SceneCache;
} // namespace SceneLayer

namespace Helpers
{
    class EntityLoader
    {
    public:
        EntityLoader(const std::string& filepath);

        std::shared_ptr<SceneLayer::Entity> LoadEntity(SceneLayer::SceneCache* sceneCache, SceneLayer::Entity* parent = nullptr);

    private:
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Transformation>& component);
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Material>& component);
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Mesh>& component);
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Light>& component);
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Skybox>& component);
        void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Tag>& component);

        void LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent);

        const std::string _entityFilepath;
        const std::string _parentFilepath;
    };
} // namespace Helpers
