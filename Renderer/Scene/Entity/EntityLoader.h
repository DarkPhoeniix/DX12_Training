#pragma once

namespace scene
{
    class Entity;
    class SceneCache;

    class Animation;
    class Armature;
    class Light;
    class Material;
    class Mesh;
    class Skybox;
    class Transformation;

    namespace Helpers
    {
        class EntityLoader
        {
        public:
            EntityLoader(const std::string& filepath);

            std::shared_ptr<scene::Entity> LoadEntity(scene::SceneCache* sceneCache, scene::Entity* parent = nullptr);

        private:
            void LoadComponent(Json::Value& jsonValue, Armature* armature, const std::shared_ptr<Animation>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Armature>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Transformation>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Material>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Mesh>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Light>& component);
            void LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Skybox>& component);

            void LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent);

            const std::string _entityFilepath;
            const std::string _parentFilepath;
        };
    } // namespace Helpers
} // namespace scene
