#pragma once

class TaskGPU;

namespace dx12
{
    class CommandList;
}

namespace scene
{
    class Entity;
    class Scene;
    class SceneCache;

    class Animation;
    class Armature;
    class Camera;
    class Light;
    class Material;
    class Mesh;
    class Skybox;
    class Transformation;
}

namespace scene::helpers
{
    class SceneLoader
    {
    public:
        void LoadScene(TaskGPU& task, const std::string& filepath, std::shared_ptr<Scene> scene);

    private:
        std::shared_ptr<scene::Entity> LoadEntity(dx12::CommandList& commandList, const std::string& filepath, scene::Entity* parent = nullptr);

        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> armature, const std::shared_ptr<Animation>& component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Armature>& component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Camera>& component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Transformation>& component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Material>& component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Mesh>& component, dx12::CommandList& commandList);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Light>& component, const std::string& name);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Skybox>& component);

        void LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent);

        void CleanIntermediates();

        scene::SceneCache* _cache;
        std::vector<dx12::Resource> _intermediates;
    };
}
