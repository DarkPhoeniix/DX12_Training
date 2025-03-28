#include "RendererPCH.h"

#include "SceneLoader.h"

#include "CommandList.h"

#include "Render/Frame/TaskGPU.h"

#include "Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Components/Transformation.h"

#include "Scene/Volumes/AABBVolume.h"
#include "Scene/Volumes/OBBVolume.h"

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

        return nominator / denominator;
    }

    void CalculateBoundingVolume(std::shared_ptr<scene::Entity> entity)
    {
        scene::Transformation* transform = entity->GetComponentAs<scene::Transformation>("Transformation");
        scene::Armature* armature = entity->GetComponentAs<scene::Armature>("Armature");
        scene::Mesh* mesh = entity->GetComponentAs<scene::Mesh>("Mesh");

        if (!mesh || !armature)
        {
            return;
        }

        auto UpdateBoneAABB = [](scene::AABBVolume& volume, scene::Bone* bone, XMFLOAT3 position)
            {
                DirectX::XMVECTOR positionVec = DirectX::XMLoadFloat3(&position);
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

            bone->AABB.Bounds = DirectX::XMMatrixScalingFromVector(boxScale) * DirectX::XMMatrixTranslationFromVector(boxLocation) * invOffset;
        }
    }
} // namespace unnamed

namespace scene::helpers
{
    std::shared_ptr<Scene> SceneLoader::LoadScene(TaskGPU& task, const std::string& filepath)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
        commandList.SetName("Scene upload command list");

        std::shared_ptr<Scene> scene = std::make_shared<Scene>();
        _cache = &scene->GetCache();

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

        scene->GetCache().GetTextureManager().UploadTextures(commandList);

        task.GetFence()->SetCompletionCallback([this]() { CleanIntermediates(); });

        commandList.Close();

        return scene;
    }

    std::shared_ptr<Entity> SceneLoader::LoadEntity(dx12::CommandList& commandList, const std::string& filepath, Entity* parent)
    {
        LOG_INFO("Parsing node " + filepath);

        if (ASSERT(std::filesystem::exists(std::filesystem::path(filepath)), std::format("Failed to parse a node from {}", filepath)))
        {
            return nullptr;
        }

        std::shared_ptr<Entity> entity = std::make_shared<Entity>(_cache, parent);

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

        CalculateBoundingVolume(entity);

        return entity;
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, Armature* armature, const std::shared_ptr<Animation>& component)
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

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Armature>& component)
    {
        std::string armatureFilepth = filepath + '/' + jsonValue["Armature"].asString();

        std::ifstream file(armatureFilepth, std::ios_base::in | std::ios_base::binary);
        Json::Value armatureData;
        file >> armatureData;

        std::string name = "Armature";
        component->SetName(name);

        std::vector<Bone> bones;
        ParseBones(armatureData["Armature"], bones);

        component->Init(bones);
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Transformation>& component)
    {
        component->Transform = ParseMatrix(jsonValue["Transform"]);
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Material>& component)
    {
        std::string materialFilepath = filepath + '/' + jsonValue["Material"].asString();

        std::ifstream file(materialFilepath, std::ios_base::in | std::ios_base::binary);
        Json::Value materialData;
        file >> materialData;

        std::string albedoFilepath = filepath + '/' + materialData["Albedo"].asString();
        std::string normalFilepath = filepath + '/' + materialData["Normal"].asString();
        std::string metalnessFilepath = filepath + '/' + materialData["Metalness"].asString();
        std::string roughnessFilepath = filepath + '/' + materialData["Roughness"].asString();

        _cache->GetTextureManager().EnqueueTexture(albedoFilepath);
        _cache->GetTextureManager().EnqueueTexture(normalFilepath);
        _cache->GetTextureManager().EnqueueTexture(metalnessFilepath);
        _cache->GetTextureManager().EnqueueTexture(roughnessFilepath);

        component->Albedo    = materialData["Albedo"].asString();
        component->NormalMap = materialData["Normal"].asString();
        component->Metalness = materialData["Metalness"].asString();
        component->Roughness = materialData["Roughness"].asString();
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Mesh>& component, dx12::CommandList& commandList)
    {
        std::string meshFilepth = filepath + '/' + jsonValue["Mesh"].asString();

        LoadRawMesh(meshFilepth, component);

        for (const VertexData& vertex : component->VertexData)
        {
            XMVECTOR position = XMLoadFloat3(&vertex.Position);

            component->LocalAABB.Min = XMVectorMin(component->LocalAABB.Min, position);
            component->LocalAABB.Max = XMVectorMax(component->LocalAABB.Max, position);
        }

        auto UploadData = [&](dx12::CommandList& commandList, dx12::Resource& destination, std::uint32_t numElements, std::uint32_t elementSize, const void* data)
            {
                std::uint32_t bufferSize = numElements * elementSize;

                dx12::ResourceDescription desc;
                {
                    desc.SetSize({ bufferSize, 1 });
                    desc.SetResourceType(dx12::ResourceType::Buffer);
                }
                destination.CreateCommitedResource(desc);

                desc.AddResourceType(dx12::ResourceType::Dynamic);
                _intermediates.emplace_back(desc);
                _intermediates.back().CreateCommitedResource();

                D3D12_SUBRESOURCE_DATA subresourceData = {};
                subresourceData.pData = data;
                subresourceData.RowPitch = bufferSize;
                subresourceData.SlicePitch = subresourceData.RowPitch;

                UpdateSubresources(commandList.GetDXCommandList().Get(),
                    destination.GetDXResource().Get(), _intermediates.back().GetDXResource().Get(),
                    0, 0, 1, &subresourceData);
            };

        // Upload Vertex buffer
        {
            component->VertexBuffer = std::make_shared<dx12::Resource>();
            UploadData(commandList, *component->VertexBuffer, component->VertexData.size(), sizeof(VertexData), component->VertexData.data());
            component->VertexBuffer->SetName(jsonValue["Mesh"].asString() + "_VB");

            component->VertexBufferView.BufferLocation = component->VertexBuffer->OffsetGPU(0);
            component->VertexBufferView.SizeInBytes = static_cast<UINT>(component->VertexData.size() * sizeof(component->VertexData[0]));
            component->VertexBufferView.StrideInBytes = sizeof(VertexData);
        }

        // Upload Skinning Vertex buffer
        if (!component->SkinningVertexData.empty())
        {
            component->SkinningVertexBuffer = std::make_shared<dx12::Resource>();
            UploadData(commandList, *component->SkinningVertexBuffer, component->SkinningVertexData.size(), sizeof(SkinningVertexData), component->SkinningVertexData.data());
            component->SkinningVertexBuffer->SetName(jsonValue["Mesh"].asString() + "_SVB");

            component->SkinningVertexBufferView.BufferLocation = component->SkinningVertexBuffer->OffsetGPU(0);
            component->SkinningVertexBufferView.SizeInBytes = static_cast<UINT>(component->SkinningVertexData.size() * sizeof(component->SkinningVertexData[0]));
            component->SkinningVertexBufferView.StrideInBytes = sizeof(SkinningVertexData);
        }

        // Upload Index buffer
        {
            component->IndexBuffer = std::make_shared<dx12::Resource>();
            UploadData(commandList, *component->IndexBuffer, component->IndexData.size(), sizeof(UINT), component->IndexData.data());
            component->IndexBuffer->SetName(jsonValue["Mesh"].asString() + "_IB");

            component->IndexBufferView.BufferLocation = component->IndexBuffer->OffsetGPU(0);
            component->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
            component->IndexBufferView.SizeInBytes = static_cast<UINT>(component->IndexData.size() * sizeof(component->IndexData[0]));
        }
    }

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Light>& component, const std::string& name)
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

    void SceneLoader::LoadComponent(const std::string& filepath, Json::Value& jsonValue, const std::shared_ptr<Skybox>& component)
    {
        std::string skyboxFilepath = filepath + '/' + jsonValue["Skybox"].asString();

        _cache->GetTextureManager().EnqueueTexture(skyboxFilepath);

        component->SkydomeTexture = jsonValue["Skybox"].asString();
    }

    void SceneLoader::LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent)
    {
        std::vector<XMFLOAT3> points;
        std::vector<XMUINT4> groupIndexes;
        std::vector<XMFLOAT4> groupWeights;
        std::vector<XMFLOAT3> normals;
        std::vector<XMFLOAT4> colors;
        std::vector<XMFLOAT2> UVs;
        std::vector<XMFLOAT3> tangents;
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
                XMFLOAT3 v;
                in >> v.x >> v.y >> v.z;
                points.push_back(v);
            }
            else if (input == "vn")
            {
                XMFLOAT3 vn;
                in >> vn.x >> vn.y >> vn.z;
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
                XMFLOAT3 tangent;
                in >> tangent.x >> tangent.y >> tangent.z;
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
        _cache->GetTextureManager().CleanIntermediates();
    }
}
