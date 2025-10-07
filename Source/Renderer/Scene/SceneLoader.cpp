#include "RendererPCH.h"

#include "SceneLoader.h"

#include "CommandList.h"
#include "ResourceBarrier.h"

#include "Render/Frame/TaskGPU.h"

#include "Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Components/Transformation.h"
#include "Scene/Volumes/AABBVolume.h"
#include "Scene/Volumes/OBBVolume.h"

#include <fstream>

using namespace DirectX;

namespace
{
    XMVECTOR ParseVector(const std::string& str)
    {
        std::stringstream iss(str);
        XMFLOAT4 r;
        iss >> r.x >> r.y >> r.z >> r.w;

        return XMLoadFloat4(&r);
    }

    XMMATRIX ParseMatrix(Json::Value& value)
    {
        XMMATRIX matrix = XMMatrixIdentity();

        for (int i = 0; i < value.size(); ++i)
        {
            matrix.r[i] = ParseVector(value[std::format("r{}", i).c_str()].asString());
        }

        return matrix;
    }

    void ParseBones(Json::Value& jsonValue, std::vector<scene::Bone>& bones, scene::BoneId ParentId = -1)
    {
        int size = jsonValue.size();
        for (int i = 0; i < size; ++i)
        {
            Json::Value& boneValue = jsonValue[i];

            scene::Bone bone;
            bone.ID = boneValue["ID"].asUInt();
            bone.Name = boneValue["Name"].asString();
            bone.ParentId = ParentId;
            bone.Offset = ParseMatrix(boneValue["Offset"]);
            bone.LocalTransform = XMMatrixIdentity();

            bones.push_back(std::move(bone));

            ParseBones(boneValue["Children"], bones, bone.ID);
        }
    }

    float CalculateDistanceToLine(const XMVECTOR& point, const XMVECTOR& lineStart, const XMVECTOR& lineEnd)
    {
        XMVECTOR v1 = XMVectorSubtract(point, lineStart);
        XMVECTOR v2 = XMVectorSubtract(lineEnd, lineStart);

        float nominator = XMVectorGetX(XMVector3Length(XMVector3Cross(v1, v2)));
        float denominator = XMVectorGetX(XMVector3Length(v2));

        ASSERT(denominator > 0.00001f, "How does it even happened?");

        return nominator / denominator;
    }

    void CalculateBoundingVolume(std::shared_ptr<scene::Entity> entity)
    {
        std::shared_ptr<scene::Transformation> transform = entity->GetComponentAs<scene::Transformation>("Transformation");
        std::shared_ptr<scene::Armature> armature = entity->GetComponentAs<scene::Armature>("Armature");
        std::shared_ptr<scene::Mesh> mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

        if (!mesh || !armature)
        {
            return;
        }

        auto UpdateBoneAABB = [](scene::AABBVolume& volume, scene::Bone* bone, XMFLOAT4 position)
            {
                DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat4(&position);
                positionVec = DirectX::XMVector4Transform(positionVec, bone->Offset);

                volume.Min = DirectX::XMVectorMin(volume.Min, positionVec);
                volume.Max = DirectX::XMVectorMax(volume.Max, positionVec);
            };

        std::vector<scene::AABBVolume> volumes(armature->GetBones().size());

        for (int i = 0; i < mesh->VertexData.size(); ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                if (mesh->SkinningVertexData[i].BoneWeights[j] > 0.00001f)
                {
                    scene::BoneId boneId = mesh->SkinningVertexData[i].BoneIds[j];
                    scene::Bone* bone = armature->GetSortedBones()[boneId];

                    UpdateBoneAABB(volumes[boneId], bone, mesh->VertexData[i].Position);
                }
            }
        }

        for (int i = 0; i < armature->GetBones().size(); ++i)
        {
            scene::Bone* bone = armature->GetSortedBones()[i];

            DirectX::XMVECTOR boxScale = DirectX::XMVectorSubtract(volumes[i].Max, volumes[i].Min) * 0.5f;
            DirectX::XMVECTOR boxLocation = DirectX::XMVectorAdd(volumes[i].Max, volumes[i].Min) * 0.5f;
            DirectX::XMMATRIX invOffset = DirectX::XMMatrixInverse(nullptr, bone->Offset);
            invOffset.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

            bone->OBB.Bounds = DirectX::XMMatrixScalingFromVector(boxScale) * DirectX::XMMatrixTranslationFromVector(boxLocation) * invOffset;
        }
    }
} // namespace unnamed

namespace scene::helpers
{
    SceneLoader::SceneLoader()
        : _resourceTable(nullptr)
        , _textureManager(nullptr)
    {
    }

    void SceneLoader::Init(ResourceTable& resourceTable, TextureManager& textureManager)
    {
        _resourceTable = &resourceTable;
        _textureManager = &textureManager;
    }

    void SceneLoader::LoadScene(TaskGPU& task, const std::string& filepath, std::shared_ptr<Scene> scene)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Scene upload command list");

        scene->Clear();

        std::ifstream in(filepath, std::ifstream::in | std::ifstream::binary);

        Json::Value root;
        in >> root;

        scene->SetName(root["Name"].asString());

        // Parse children nodes
        for (auto& node : root["Nodes"])
        {
            std::string nodePath = std::filesystem::path(filepath).parent_path().string() + '/' + node.asString();

            scene->AddRootNode(LoadEntity(commandList, nodePath));
        }

        _textureManager->UploadTextures(commandList);

        task.GetFence()->SetCompletionCallback([this]() { CleanIntermediates(); });

        LOG_INFO("Loading scene: {}", filepath);
    }

    std::shared_ptr<dx12::Resource> SceneLoader::GenerateEnvironmentDiffuseIrradianceMap(dx12::CommandList& commandList, std::shared_ptr<Scene> scene)
    {
        // Find skybox node and retrieve texture pointer

        std::shared_ptr<scene::Entity> skyboxNode = scene->FindNodeByComponentName("Skybox");
        ASSERT(skyboxNode, "Failed to get skybox node");

        std::shared_ptr<scene::Skybox> skyboxComponent = skyboxNode->GetComponentAs<scene::Skybox>("Skybox");
        ASSERT(skyboxComponent, "Failed to get skybox component");

        std::shared_ptr<dx12::Resource> skyboxTexture = _textureManager->GetTexture(skyboxComponent->SkydomeTextureHandle);
        ASSERT(skyboxTexture, "Failed to get skybox texture pointer");

        // Parse pipeline for the diffuse irradiance convolution

        _IBL_DiffuseIrradianceConvolution.Parse("PipelineDescriptions\\IBL_DiffuseIrradianceConvolution.tech");

        // Create diffuse irradiance map texture

        dx12::ResourceDescription diffuseIrradianceTextureDesc;
        {
            diffuseIrradianceTextureDesc.SetSize({ 128, 128 });
            diffuseIrradianceTextureDesc.SetDepthOrArraySize(6);
            diffuseIrradianceTextureDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            diffuseIrradianceTextureDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        std::shared_ptr<dx12::Resource> diffuseIrradianceMap = ResourceFactory::Create("diffuse_irradiance_map", diffuseIrradianceTextureDesc);
        diffuseIrradianceMap->CreateCommitedResource();

        // Create SRV/UAV for the textures

        DescriptorHandle skyboxTextureHandle = _resourceTable->GetStaticResourceHandle(skyboxTexture->GetAsSRV());
        DescriptorHandle diffuseIrradianceMapHandle = _resourceTable->AddStaticResourceView(diffuseIrradianceMap->GetAsUAV());

        // Transition resources

        std::vector<dx12::ResourceBarrier> barriers =
        {
            { skyboxTexture, dx12::ResourceState::Common, dx12::ResourceState::NonPixelShaderResource },
            { diffuseIrradianceMap, dx12::ResourceState::Common, dx12::ResourceState::UnorderedAccess }
        };
        commandList.TransitionBarriers(barriers);

        // Diffuse irradiance convolution pipeline

        commandList.SetDescriptorHeaps({ _resourceTable->GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
        commandList.SetPipelineState(_IBL_DiffuseIrradianceConvolution);

        struct PassConstants
        {
            std::uint32_t SkyboxTextureIndex;
            std::uint32_t DiffuseIrradianceMapIndex;
        } passCB{ .SkyboxTextureIndex = skyboxTextureHandle.Index, .DiffuseIrradianceMapIndex = diffuseIrradianceMapHandle.Index };
        commandList.SetConstants(1, 2, &passCB);

        std::uint32_t xThreadGroups = (uint32_t)std::ceilf(diffuseIrradianceTextureDesc.GetSize().x / 8.0f);
        std::uint32_t yThreadGroups = (uint32_t)std::ceilf(diffuseIrradianceTextureDesc.GetSize().y / 8.0f);
        std::uint32_t zThreadGroups = 6; // One per each cube face

        commandList.Dispatch(xThreadGroups, yThreadGroups, zThreadGroups);

        // Transition resources

        barriers =
        {
            { skyboxTexture, dx12::ResourceState::NonPixelShaderResource, dx12::ResourceState::Common },
            { diffuseIrradianceMap, dx12::ResourceState::UnorderedAccess, dx12::ResourceState::NonPixelShaderResource }
        };
        commandList.TransitionBarriers(barriers);

        return diffuseIrradianceMap;
    }

    std::shared_ptr<dx12::Resource> SceneLoader::GeneratePreFilteredEnvironmentMap(dx12::CommandList& commandList, std::shared_ptr<Scene> scene)
    {
        // Find skybox node and retrieve texture pointer

        std::shared_ptr<scene::Entity> skyboxNode = scene->FindNodeByComponentName("Skybox");
        ASSERT(skyboxNode, "Failed to get skybox node");

        std::shared_ptr<scene::Skybox> skyboxComponent = skyboxNode->GetComponentAs<scene::Skybox>("Skybox");
        ASSERT(skyboxComponent, "Failed to get skybox component");

        std::shared_ptr<dx12::Resource> skyboxTexture = _textureManager->GetTexture(skyboxComponent->SkydomeTextureHandle);
        ASSERT(skyboxTexture, "Failed to get skybox texture pointer");

        // Parse pipeline for the environment map pre-filtering

        _IBL_PreFilterEnvMap.Parse("PipelineDescriptions\\IBL_PreFilterEnvMap.tech");

        // Create environment map pre-filtered texture

        dx12::ResourceDescription preFilteredEnvTextureDesc;
        {
            preFilteredEnvTextureDesc.SetSize({ 512, 512 });
            preFilteredEnvTextureDesc.SetDepthOrArraySize(6);
            preFilteredEnvTextureDesc.SetMipLevels(6);
            preFilteredEnvTextureDesc.SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
            preFilteredEnvTextureDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        std::shared_ptr<dx12::Resource> preFilteredEnvMap = ResourceFactory::Create("prefiltered_environment_map", preFilteredEnvTextureDesc);
        preFilteredEnvMap->CreateCommitedResource();

        // Create SRV/UAV for the textures

        DescriptorHandle skyboxTextureHandle = _resourceTable->GetStaticResourceHandle(skyboxTexture->GetAsSRV());

        // Transition resources

        std::vector<dx12::ResourceBarrier> barriers =
        {
            { skyboxTexture, dx12::ResourceState::Common, dx12::ResourceState::NonPixelShaderResource },
            { preFilteredEnvMap, dx12::ResourceState::Common, dx12::ResourceState::UnorderedAccess }
        };
        commandList.TransitionBarriers(barriers);

        // Environment pre-filtering pipeline

        commandList.SetDescriptorHeaps({ _resourceTable->GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
        commandList.SetPipelineState(_IBL_PreFilterEnvMap);

        // Generate each mip level
        for (std::uint32_t i = 0; i < preFilteredEnvTextureDesc.GetMipLevels(); ++i)
        {
            dx12::UnorderedAccessView uav;
            {
                uav.Owner = preFilteredEnvMap;
                uav.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                uav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

                uav.Texture2DArray.ArraySize = 6;
                uav.Texture2DArray.MipSlice = i;
                uav.Texture2DArray.PlaneSlice = 0;
                uav.Texture2DArray.FirstArraySlice = 0;
            }
            DescriptorHandle preFilteredEnvironmentMapHandle = _resourceTable->AddStaticResourceView(uav);

            float roughness = float(i) / float(preFilteredEnvTextureDesc.GetMipLevels() - 1);

            struct PassConstants
            {
                float Roughness;
                std::uint32_t SkyboxTextureIndex;
                std::uint32_t PreFilteredEnvironmentMap;
            } passCB{ .Roughness = roughness, .SkyboxTextureIndex = skyboxTextureHandle.Index, .PreFilteredEnvironmentMap = preFilteredEnvironmentMapHandle.Index };
            commandList.SetConstants(1, 3, &passCB);

            std::uint32_t xThreadGroups = (uint32_t)std::ceilf(preFilteredEnvTextureDesc.GetSize().x / (8.0f * std::pow(2, i)));
            std::uint32_t yThreadGroups = (uint32_t)std::ceilf(preFilteredEnvTextureDesc.GetSize().y / (8.0f * std::pow(2, i)));
            std::uint32_t zThreadGroups = 6; // One per each cube face

            commandList.Dispatch(xThreadGroups, yThreadGroups, zThreadGroups);
        }

        // Transition resources

        barriers =
        {
            { skyboxTexture, dx12::ResourceState::NonPixelShaderResource, dx12::ResourceState::Common },
            { preFilteredEnvMap, dx12::ResourceState::UnorderedAccess, dx12::ResourceState::NonPixelShaderResource }
        };
        commandList.TransitionBarriers(barriers);

        return preFilteredEnvMap;
    }

    std::shared_ptr<dx12::Resource> SceneLoader::GenerateEnvironmentBRDFLookUpTexture(dx12::CommandList& commandList, std::shared_ptr<Scene> scene)
    {
        // Find skybox node and retrieve texture pointer

        std::shared_ptr<scene::Entity> skyboxNode = scene->FindNodeByComponentName("Skybox");
        ASSERT(skyboxNode, "Failed to get skybox node");

        std::shared_ptr<scene::Skybox> skyboxComponent = skyboxNode->GetComponentAs<scene::Skybox>("Skybox");
        ASSERT(skyboxComponent, "Failed to get skybox component");

        std::shared_ptr<dx12::Resource> skyboxTexture = _textureManager->GetTexture(skyboxComponent->SkydomeTextureHandle);
        ASSERT(skyboxTexture, "Failed to get skybox texture pointer");

        // Parse pipeline for BRDF look-up texture generation

        _IBL_BRDFGenerateLUT.Parse("PipelineDescriptions\\IBL_BRDFGenerateLUT.tech");

        // Create BRDF look-up texture

        dx12::ResourceDescription brdfLUTDesc;
        {
            brdfLUTDesc.SetSize({ 512, 512 });
            brdfLUTDesc.SetFormat(DXGI_FORMAT_R16G16_FLOAT);
            brdfLUTDesc.SetResourceType(dx12::ResourceType::Texture | dx12::ResourceType::Unordered);
        }
        std::shared_ptr<dx12::Resource> brdfLUT = ResourceFactory::Create("brdf_lut", brdfLUTDesc);
        brdfLUT->CreateCommitedResource();

        // Create SRV/UAV for the textures

        DescriptorHandle brdfLUTTextureHandle = _resourceTable->AddStaticResourceView(brdfLUT->GetAsUAV());

        // Transition resources

        std::vector<dx12::ResourceBarrier> barriers =
        {
            { skyboxTexture, dx12::ResourceState::Common, dx12::ResourceState::NonPixelShaderResource },
            { brdfLUT, dx12::ResourceState::Common, dx12::ResourceState::UnorderedAccess }
        };
        commandList.TransitionBarriers(barriers);

        // BRDF LUT generation pipeline

        commandList.SetDescriptorHeaps({ _resourceTable->GetShaderResourcesDescriptorHeap().GetDXDescriptorHeap().Get() });
        commandList.SetPipelineState(_IBL_BRDFGenerateLUT);

        struct PassConstants
        {
            std::uint32_t brdfLUTTextureIndex;
        } passCB{ .brdfLUTTextureIndex = brdfLUTTextureHandle.Index };
        commandList.SetConstants(1, 3, &passCB);

        std::uint32_t xThreadGroups = (uint32_t)std::ceilf(brdfLUTDesc.GetSize().x / 8.0f);
        std::uint32_t yThreadGroups = (uint32_t)std::ceilf(brdfLUTDesc.GetSize().y / 8.0f);

        commandList.Dispatch(xThreadGroups, yThreadGroups);

        // Transition resources

        barriers =
        {
            { skyboxTexture, dx12::ResourceState::NonPixelShaderResource, dx12::ResourceState::Common },
            { brdfLUT, dx12::ResourceState::UnorderedAccess, dx12::ResourceState::NonPixelShaderResource }
        };
        commandList.TransitionBarriers(barriers);
        commandList.UAVBarrier(brdfLUT);

        return brdfLUT;
    }

    std::shared_ptr<Entity> SceneLoader::LoadEntity(dx12::CommandList& commandList, const std::string& filepath, Entity* parent)
    {
        LOG_INFO("Parsing node: {}", filepath);

        ASSERT(std::filesystem::exists(std::filesystem::path(filepath)), "Failed to parse a node from " + filepath);

        std::shared_ptr<Entity> entity = std::make_shared<Entity>(parent);

        std::ifstream in(filepath, std::ifstream::in | std::ifstream::binary);
        Json::Value jsonRoot;
        in >> jsonRoot;

        // Parse name
        entity->SetName(jsonRoot["Name"].asCString());

        // Parse children nodes
        for (auto& node : jsonRoot["Children"])
        {
            std::string nodeFilepath = filepath + '/' + node.asString();

            std::shared_ptr<Entity> childEntity = LoadEntity(commandList, nodeFilepath, entity.get());

            entity->AddChild(childEntity);
        }

        std::string parentPath = std::filesystem::path(filepath).parent_path().string();

        if (!jsonRoot["Transform"].isNull())
        {
            std::shared_ptr<Transformation> component = std::make_shared<Transformation>();
            LoadComponent(parentPath, jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Material"].isNull())
        {
            std::shared_ptr<Material> component = std::make_shared<Material>();
            LoadComponent(parentPath, jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Mesh"].isNull())
        {
            std::shared_ptr<Mesh> component = std::make_shared<Mesh>();
            LoadComponent(parentPath, jsonRoot, component, commandList);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Light"].isNull())
        {
            std::shared_ptr<Light> component = std::make_shared<Light>();
            LoadComponent(parentPath, jsonRoot, component, entity->GetName());
            entity->AddComponent(component);
        }
        if (!jsonRoot["Skybox"].isNull())
        {
            std::shared_ptr<Skybox> component = std::make_shared<Skybox>();
            LoadComponent(parentPath, jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Armature"].isNull())
        {
            std::shared_ptr<Armature> component = std::make_shared<Armature>();
            LoadComponent(parentPath, jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Animation"].isNull())
        {
            std::shared_ptr<Animation> component = std::make_shared<Animation>();
            LoadComponent(parentPath, jsonRoot, entity->GetComponentAs<Armature>("Armature"), component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Camera"].isNull())
        {
            std::shared_ptr<Camera> component = std::make_shared<Camera>();
            LoadComponent(parentPath, jsonRoot, component);
            entity->AddComponent(component);
        }

        CalculateBoundingVolume(entity);

        return entity;
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> armature, std::shared_ptr<Animation> component)
    {
        std::string animationFilepth = filepath + '/' + jsonValue["Animation"].asString();

        std::ifstream file(animationFilepth, std::ios_base::in | std::ios_base::binary);
        Json::Value animationData;
        file >> animationData;

        component->Name = jsonValue["Animation"].asString();

        int framesNum = animationData["Frames"].size();
        for (int frameIndex = 0; frameIndex < framesNum; ++frameIndex)
        {
            AnimationFrame frame;
            frame.Index = frameIndex;

            std::string frameNumStr = std::to_string(frameIndex);
            std::uint32_t boneIndex = 0;

            for (Json::Value::const_iterator frameIt = animationData["Frames"][frameNumStr].begin(); frameIt != animationData["Frames"][frameNumStr].end(); frameIt++)
            {
                std::string name = frameIt.key().asString();
                Bone* b = armature->GetBoneByName(name);

                frame.Locations[b->ID] = ParseVector(animationData["Frames"][frameNumStr][frameIt.key().asString()]["LocationVec"].asString());
                frame.Rotations[b->ID] = ParseVector(animationData["Frames"][frameNumStr][frameIt.key().asString()]["RotationQuat"].asString());
            }

            component->Frames.push_back(frame);
        }

        component->TicksPerSecond = animationData["FrameRate"].asFloat();
        component->Duration = animationData["Duration"].asFloat();
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Armature> component)
    {
        std::string armatureFilepath = filepath + '/' + jsonValue["Armature"].asString();

        std::ifstream file(armatureFilepath, std::ios_base::in | std::ios_base::binary);
        Json::Value armatureData;
        file >> armatureData;

        std::string name = "Armature";
        component->SetName(name);

        std::vector<Bone> bones;
        ParseBones(armatureData["Armature"], bones);

        component->Init(bones);
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Camera> component)
    {
        std::string cameraFilepath = filepath + '/' + jsonValue["Camera"].asString();

        std::ifstream file(cameraFilepath, std::ios_base::in | std::ios_base::binary);
        Json::Value cameraData;
        file >> cameraData;

        component->LookAt(ParseVector(cameraData["Position"].asString()), ParseVector(cameraData["Target"].asString()), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
        component->SetLens(cameraData["FOV"].asFloat(), cameraData["NearZ"].asFloat(), cameraData["FarZ"].asFloat());
        component->Speed = cameraData["Speed"].asFloat();
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Transformation> component)
    {
        component->Transform = ParseMatrix(jsonValue["Transform"]);
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Material> component)
    {
        std::string materialFilepath = filepath + '/' + jsonValue["Material"].asString();

        std::ifstream file(materialFilepath, std::ios_base::in | std::ios_base::binary);
        Json::Value materialData;
        file >> materialData;

        if (const Json::Value& albedoTextureNode = materialData["AlbedoTexture"]; !albedoTextureNode.isNull())
        {
            std::string albedoFilepath = filepath + '/' + albedoTextureNode.asString();
            component->AlbedoTextureHandle = _textureManager->EnqueueTexture(albedoFilepath);
        }
        if (const Json::Value& albedoNode = materialData["Albedo"]; !albedoNode.isNull())
        {
            component->AlbedoColor = ParseVector(albedoNode.asString());
        }
        if (const Json::Value& normalTextureNode = materialData["NormalTexture"]; !normalTextureNode.isNull())
        {
            std::string normalFilepath = filepath + '/' + normalTextureNode.asString();
            component->NormalMapTextureHandle = _textureManager->EnqueueTexture(normalFilepath);
        }
        if (const Json::Value& metalnessTextureNode = materialData["MetallicTexture"]; !metalnessTextureNode.isNull())
        {
            std::string metalnessFilepath = filepath + '/' + metalnessTextureNode.asString();
            component->MetalnessTextureHandle = _textureManager->EnqueueTexture(metalnessFilepath);
        }
        if (const Json::Value& metalnessValueNode = materialData["Metallic"]; !metalnessValueNode.isNull())
        {
            component->MetallicValue = metalnessValueNode.asFloat();
        }
        if (const Json::Value& roughnessNode = materialData["RoughnessTexture"]; !roughnessNode.isNull())
        {
            std::string roughnessFilepath = filepath + '/' + roughnessNode.asString();
            component->RoughnessTextureHandle = _textureManager->EnqueueTexture(roughnessFilepath);
        }
        if (const Json::Value& roughnessValueNode = materialData["Roughness"]; !roughnessValueNode.isNull())
        {
            component->RoughnessValue = roughnessValueNode.asFloat();
        }
        if (const Json::Value& emissionNode = materialData["EmissionTexture"]; !emissionNode.isNull())
        {
            std::string emissionFilepath = filepath + '/' + emissionNode.asString();
            component->EmissionTextureHandle = _textureManager->EnqueueTexture(emissionFilepath);
        }
        if (const Json::Value& emissionColorNode = materialData["EmissionColor"]; !emissionColorNode.isNull())
        {
            component->EmissionColor = ParseVector(emissionColorNode.asString());
        }
        if (const Json::Value& emissionIntensityNode = materialData["EmissionIntensity"]; !emissionIntensityNode.isNull())
        {
            component->EmissionIntensity = emissionIntensityNode.asFloat();
        }
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Mesh> component, dx12::CommandList& commandList)
    {
        std::string meshFilepth = filepath + '/' + jsonValue["Mesh"].asString();

        LoadRawMesh(meshFilepth, component);

        for (const VertexData& vertex : component->VertexData)
        {
            XMVECTOR position = XMLoadFloat4(&vertex.Position);

            component->LocalAABB.Min = XMVectorMin(component->LocalAABB.Min, position);
            component->LocalAABB.Max = XMVectorMax(component->LocalAABB.Max, position);
        }

        auto UploadData = [&](dx12::CommandList& commandList, std::uint32_t numElements, std::uint32_t elementSize, const void* data)
            {
                std::uint32_t bufferSize = numElements * elementSize;

                dx12::ResourceDescription desc;
                {
                    desc.SetSize({ bufferSize, 1 });
                    desc.SetResourceType(dx12::ResourceType::Buffer);
                }
                std::shared_ptr<dx12::Resource> destination = ResourceFactory::Create("", desc);
                destination->CreateCommitedResource();

                desc.AddResourceType(dx12::ResourceType::Dynamic);

                std::shared_ptr<dx12::Resource> intermediate = ResourceFactory::Create("Intermediate", desc);
                intermediate->CreateCommitedResource();
                _intermediates.push_back(intermediate);

                D3D12_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pData = data;
                subresourceData.RowPitch = bufferSize;
                subresourceData.SlicePitch = subresourceData.RowPitch;

                UpdateSubresources(commandList.GetDXCommandList().Get(),
                    destination->GetDXResource().Get(), _intermediates.back()->GetDXResource().Get(),
                    0, 0, 1, &subresourceData);

                return destination;
            };

        // Upload Vertex buffer
        {
            component->VertexBuffer = UploadData(commandList, component->VertexData.size(), sizeof(VertexData), component->VertexData.data());
            component->VertexBuffer->SetName(jsonValue["Mesh"].asString() + "_VB");

            component->VertexBufferView.BufferLocation = component->VertexBuffer->OffsetGPU(0);
            component->VertexBufferView.SizeInBytes = static_cast<UINT>(component->VertexData.size() * sizeof(VertexData));
            component->VertexBufferView.StrideInBytes = sizeof(VertexData);
        }

        // Upload Skinning Vertex buffer
        if (!component->SkinningVertexData.empty())
        {
            component->SkinningVertexBuffer = UploadData(commandList, component->SkinningVertexData.size(), sizeof(SkinningVertexData), component->SkinningVertexData.data());
            component->SkinningVertexBuffer->SetName(jsonValue["Mesh"].asString() + "_SVB");

            component->SkinningVertexBufferView.BufferLocation = component->SkinningVertexBuffer->OffsetGPU(0);
            component->SkinningVertexBufferView.SizeInBytes = static_cast<UINT>(component->SkinningVertexData.size() * sizeof(component->SkinningVertexData[0]));
            component->SkinningVertexBufferView.StrideInBytes = sizeof(SkinningVertexData);
        }

        // Upload Index buffer
        {
            component->IndexBuffer = UploadData(commandList, component->IndexData.size(), sizeof(UINT), component->IndexData.data());
            component->IndexBuffer->SetName(jsonValue["Mesh"].asString() + "_IB");

            component->IndexBufferView.BufferLocation = component->IndexBuffer->OffsetGPU(0);
            component->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
            component->IndexBufferView.SizeInBytes = static_cast<UINT>(component->IndexData.size() * sizeof(component->IndexData[0]));
        }
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Light> component, const std::string& name)
    {
        std::string lightFilepth = filepath + '/' + jsonValue["Light"].asString();

        std::ifstream file(lightFilepth, std::ios_base::in | std::ios_base::binary);
        Json::Value lightData;
        file >> lightData;

        std::string lightType = lightData["Type"].asString();

        if (lightType == "Directional")
        {
            component->Type = LightType::Directional;
        }
        else if (lightType == "Point")
        {
            component->Type = LightType::Point;
        }
        else if (lightType == "Spot")
        {
            component->Type = LightType::Spot;
        }
        component->Direction = ParseVector(lightData["Direction"].asString());
        component->Color = ParseVector(lightData["Color"].asString());
        component->Intensity = lightData["Intensity"].asFloat();
        component->Range = lightData["Range"].asFloat();
        component->OuterAngle = lightData["OuterAngle"].asFloat();
        component->InnerAngle = lightData["InnerAngle"].asFloat();
        component->CastShadows = lightData["CastShadows"].asUInt();
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, std::shared_ptr<Skybox> component)
    {
        std::string skyboxFilepath = filepath + '/' + jsonValue["Skybox"].asString();

        component->SkydomeTextureHandle = _textureManager->EnqueueTexture(skyboxFilepath);
    }

    void SceneLoader::LoadRawMesh(const std::string& filepath, std::shared_ptr<Mesh> meshComponent)
    {
        std::vector<XMFLOAT4> points;
        std::vector<XMUINT4>  groupIndexes;
        std::vector<XMFLOAT4> groupWeights;
        std::vector<XMFLOAT4> normals;
        std::vector<XMFLOAT4> tangents;
        std::vector<XMFLOAT4> colors;
        std::vector<XMFLOAT2> UVs;
        UINT64 index = 0;

        std::string input;
        std::ifstream in(filepath, std::ios_base::in);
        while (!in.eof())
        {
            input = "";
            in >> input;

            if (input == "")
            {
                char line[512];
                in.getline(line, 512);
            }
            else if (input == "v")
            {
                XMFLOAT4 v;
                in >> v.x >> v.y >> v.z;
                v.w = 1.0f;
                points.push_back(v);
            }
            else if (input == "vn")
            {
                XMFLOAT4 vn;
                in >> vn.x >> vn.y >> vn.z;
                vn.w = 0.0f;
                normals.push_back(vn);
            }
            else if (input == "vc")
            {
                float r, g, b, a;
                in >> r >> g >> b >> a;
                colors.push_back({ r, g, b, a });
            }
            else if (input == "vt")
            {
                float u, v;
                in >> u >> v;
                UVs.push_back({ u, v });
            }
            else if (input == "vtan")
            {
                XMFLOAT4 tangent;
                in >> tangent.x >> tangent.y >> tangent.z >> tangent.w;
                tangents.push_back(tangent);
            }
            else if (input == "gi")
            {
                XMUINT4 groupIndex;
                in >> groupIndex.x >> groupIndex.y >> groupIndex.z >> groupIndex.w;
                groupIndexes.push_back(groupIndex);
            }
            else if (input == "gw")
            {
                XMFLOAT4 groupWeight;
                in >> groupWeight.x >> groupWeight.y >> groupWeight.z >> groupWeight.w;
                groupWeights.push_back(groupWeight);
            }
            else if (input == "f")
            {
                char sym;
                UINT64 v, vn, vt, vtan;

                for (int i = 0; i < 3; ++i)
                {
                    in >> v >> sym >> vt >> sym >> vn >> sym >> vtan;

                    VertexData vertex;
                    vertex.Position = points[v];
                    vertex.Normal = normals[vn];
                    vertex.UV = UVs[vt];
                    vertex.Tangent = tangents[vtan];

                    SkinningVertexData skin;
                    if (!groupIndexes.empty())
                    {
                        skin.BoneIds[0] = groupIndexes[v].x;
                        skin.BoneIds[1] = groupIndexes[v].y;
                        skin.BoneIds[2] = groupIndexes[v].z;
                        skin.BoneIds[3] = groupIndexes[v].w;
                        skin.BoneWeights[0] = groupWeights[v].x;
                        skin.BoneWeights[1] = groupWeights[v].y;
                        skin.BoneWeights[2] = groupWeights[v].z;
                        skin.BoneWeights[3] = groupWeights[v].w;
                    }

                    meshComponent->VertexData.push_back(vertex);
                    if (!groupIndexes.empty())
                    {
                        meshComponent->SkinningVertexData.push_back(skin);
                    }
                    meshComponent->IndexData.push_back(index++);
                }
            }
        }
    }

    void SceneLoader::CleanIntermediates()
    {
        _intermediates.clear();
        _textureManager->ClearIntermediates();
    }
}
