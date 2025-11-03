#pragma once

#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"

#include "RHI/DescriptorHeap.h"
#include "RHI/PipelineState.h"

class TaskGPU;

namespace dx12
{
    class CommandList;
    class Texture;
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
        SceneLoader();

        void Init(ResourceTable& resourceTable, TextureManager& textureManager);

        void LoadScene(TaskGPU& task, const std::string& filepath, std::shared_ptr<Scene> scene);

        std::shared_ptr<dx12::Resource> GenerateEnvironmentDiffuseIrradianceMap(dx12::CommandList& commandList, std::shared_ptr<Scene> scene);
        std::shared_ptr<dx12::Resource> GeneratePreFilteredEnvironmentMap(dx12::CommandList& commandList, std::shared_ptr<Scene> scene);
        std::shared_ptr<dx12::Resource> GenerateEnvironmentBRDFLookUpTexture(dx12::CommandList& commandList, std::shared_ptr<Scene> scene);

    private:
        std::shared_ptr<scene::Entity> LoadEntity(dx12::CommandList& commandList, const std::string& filepath, scene::Entity* parent = nullptr);

        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> armature, std::shared_ptr<Animation> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Camera> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Transformation> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Material> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Mesh> component, dx12::CommandList& commandList);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Light> component, const std::string& name);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Skybox> component);

        void LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh> meshComponent);

        void CleanIntermediates();

        dx12::PipelineState _IBL_DiffuseIrradianceConvolution;
        dx12::PipelineState _IBL_PreFilterEnvMap;
        dx12::PipelineState _IBL_BRDFGenerateLUT;

        ResourceTable* _resourceTable;
        TextureManager* _textureManager;
        std::vector<std::shared_ptr<dx12::Resource>> _intermediates;
    };
}
