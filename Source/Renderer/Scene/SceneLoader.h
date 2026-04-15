#pragma once

#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"

#include "RHI/DescriptorHeap.h"
#include "RHI/PipelineState.h"

#include <json/json.h>

class TaskGPU;

namespace rhi
{
    class CommandList;
    class Texture;
} // namespace rhi

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
}// namespace scene

namespace scene::helpers
{
    class SceneLoader
    {
    public:
        SceneLoader(rhi::Device* device);

        void Init(ResourceTable& resourceTable, TextureManager& textureManager);

        void LoadScene(TaskGPU& task, const std::string& filepath, std::shared_ptr<Scene> scene);

        std::shared_ptr<rhi::Texture> GenerateEnvironmentDiffuseIrradianceMap(rhi::CommandList* commandList, std::shared_ptr<Scene> scene);
        std::shared_ptr<rhi::Texture> GeneratePreFilteredEnvironmentMap(rhi::CommandList* commandList, std::shared_ptr<Scene> scene);
        std::shared_ptr<rhi::Texture> GenerateEnvironmentBRDFLookUpTexture(rhi::CommandList* commandList, std::shared_ptr<Scene> scene);

    private:
        std::shared_ptr<scene::Entity> LoadEntity(rhi::CommandList* commandList, const std::string& filepath, scene::Entity* parent = nullptr);

        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> armature, std::shared_ptr<Animation> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Camera> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Transformation> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Material> component);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Mesh> component, rhi::CommandList* commandList);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Light> component, const std::string& name);
        void LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Skybox> component);

        void LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh> meshComponent);

        void CleanIntermediates();

        std::unique_ptr<rhi::PipelineState> _IBL_DiffuseIrradianceConvolution;
        std::unique_ptr<rhi::PipelineState> _IBL_PreFilterEnvMap;
        std::unique_ptr<rhi::PipelineState> _IBL_BRDFGenerateLUT;

        ResourceTable* _resourceTable;
        TextureManager* _textureManager;
        std::vector<std::shared_ptr<rhi::Buffer>> _intermediates;

        rhi::Device* _device;
    };
} // namespace scene::helpers
