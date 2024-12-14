#include "stdafx.h"

#include "EntityLoader.h"

#include "Scene/ECS/Entity.h"
#include "Scene/ECS/Components/Animation.h"
#include "Scene/ECS/Components/Armature.h"
#include "Scene/ECS/Components/Light.h"
#include "Scene/ECS/Components/Material.h"
#include "Scene/ECS/Components/Mesh.h"
#include "Scene/ECS/Components/Skybox.h"
#include "Scene/ECS/Components/Transformation.h"

using namespace SceneLayer;

namespace
{
    DirectX::XMVECTOR ParseVector(const std::string& str)
    {
        std::stringstream iss(str);
        DirectX::XMFLOAT4 r;
        iss >> r.x >> r.y >> r.z >> r.w;

        return DirectX::XMLoadFloat4(&r);
    }

    DirectX::XMMATRIX ParseMatrix(Json::Value& value)
    {
        DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

        for (int i = 0; i < value.size(); ++i)
        {
            matrix.r[i] = ParseVector(value[std::format("r{}", i).c_str()].asString());
        }

        return matrix;
    }

    void ParseBones(Json::Value& jsonValue, std::vector<Bone>& bones, BoneId ParentId = -1)
    {
        int size = jsonValue.size();
        for (int i = 0; i < size; ++i)
        {
            Json::Value& boneValue = jsonValue[i];

            Bone bone;
            bone.ID = boneValue["ID"].asUInt();
            bone.Name = boneValue["Name"].asString();
            bone.ParentId = ParentId;
            bone.Offset = ParseMatrix(boneValue["Offset"]);
            bone.LocalTransform = DirectX::XMMatrixIdentity();

            bones.push_back(std::move(bone));

            ParseBones(boneValue["Children"], bones, bone.ID);
        }
    }
} // namespace unnamed

namespace Helpers
{
    EntityLoader::EntityLoader(const std::string& filepath)
        : _entityFilepath(filepath)
        , _parentFilepath(std::filesystem::path(_entityFilepath).parent_path().string())
    {
    }

    std::shared_ptr<Entity> EntityLoader::LoadEntity(SceneCache* sceneCache, Entity* parent)
    {
        LOG_INFO("Parsing node " + _entityFilepath);

        if (ASSERT(std::filesystem::exists(std::filesystem::path(_entityFilepath)), std::format("Failed to parse a node from {}", _entityFilepath)))
        {
            return nullptr;
        }

        std::shared_ptr<Entity> entity = std::make_shared<Entity>(sceneCache, parent);

        std::ifstream in(_entityFilepath, std::ifstream::in | std::ifstream::binary);
        Json::Value jsonRoot;
        in >> jsonRoot;

        // Parse name
        entity->SetName(jsonRoot["Name"].asCString());

        // Parse children nodes
        for (auto& node : jsonRoot["Children"])
        {
            std::string nodeFilepath = _parentFilepath + '/' + node.asString();

            EntityLoader loader(nodeFilepath);
            std::shared_ptr<Entity> childEntity = loader.LoadEntity(sceneCache, entity.get());

            entity->AddChild(childEntity);
        }

        if (!jsonRoot["Transform"].isNull())
        {
            std::shared_ptr<Transformation> component = std::make_shared<Transformation>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Material"].isNull())
        {
            std::shared_ptr<Material> component = std::make_shared<Material>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Mesh"].isNull())
        {
            std::shared_ptr<Mesh> component = std::make_shared<Mesh>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Light"].isNull())
        {
            std::shared_ptr<Light> component = std::make_shared<Light>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Skybox"].isNull())
        {
            std::shared_ptr<Skybox> component = std::make_shared<Skybox>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Armature"].isNull())
        {
            std::shared_ptr<Armature> component = std::make_shared<Armature>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }
        if (!jsonRoot["Animation"].isNull())
        {
            std::shared_ptr<Animation> component = std::make_shared<Animation>();
            LoadComponent(jsonRoot, entity->GetComponentAs<Armature>("Armature"), component);
            entity->AddComponent(component);
        }

        return entity;
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, Armature* armature, const std::shared_ptr<Animation>& component)
    {
        std::string animationFilepth = _parentFilepath + '/' + jsonValue["Animation"].asString();

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

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Armature>& component)
    {
        std::string armatureFilepth = _parentFilepath + '/' + jsonValue["Armature"].asString();

        std::ifstream file(armatureFilepth, std::ios_base::in | std::ios_base::binary);
        Json::Value armatureData;
        file >> armatureData;

        std::string name = "Armature";
        component->SetName(name);

        std::vector<Bone> bones;
        ParseBones(armatureData["Armature"], bones);

        component->Init(bones);
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Transformation>& component)
    {
        component->Transform = ParseMatrix(jsonValue["Transform"]);
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Material>& component)
    {
        std::string materialFilepth = _parentFilepath + '/' + jsonValue["Material"].asString();

        std::ifstream file(materialFilepth, std::ios_base::in | std::ios_base::binary);
        Json::Value materialData;
        file >> materialData;

        std::string albedoFilepath = _parentFilepath + '/' + materialData["Albedo"].asString();
        std::string normalFilepath = _parentFilepath + '/' + materialData["Normal"].asString();
        std::string metalnessFilepath = _parentFilepath + '/' + materialData["Metalness"].asString();
        std::string roughnessFilepath = _parentFilepath + '/' + materialData["Roughness"].asString();

        component->Albedo = dx12::Texture::LoadFromFile(albedoFilepath);
        component->NormalMap = dx12::Texture::LoadFromFile(normalFilepath);
        component->Metalness = dx12::Texture::LoadFromFile(metalnessFilepath);
        component->Roughness = dx12::Texture::LoadFromFile(roughnessFilepath);
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Mesh>& component)
    {
        std::string meshFilepth = _parentFilepath + '/' + jsonValue["Mesh"].asString();

        LoadRawMesh(meshFilepth, component);

        DirectX::XMFLOAT4 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1.0f);
        DirectX::XMFLOAT4 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), 1.0f);

        for (const VertexData& vertex : component->VertexData)
        {
            if (vertex.Position.x < min.x)
            {
                min.x = vertex.Position.x;
            }
            else if (vertex.Position.x > max.x)
            {
                max.x = vertex.Position.x;
            }

            if (vertex.Position.y < min.y)
            {
                min.y = vertex.Position.y;
            }
            else if (vertex.Position.y > max.y)
            {
                max.y = vertex.Position.y;
            }

            if (vertex.Position.z < min.z)
            {
                min.z = vertex.Position.z;
            }
            else if (vertex.Position.z > max.z)
            {
                max.z = vertex.Position.z;
            }
        }

        component->AABB.min = DirectX::XMLoadFloat4(&min);
        component->AABB.max = DirectX::XMLoadFloat4(&max);
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Light>& component)
    {
        std::string lightFilepth = _parentFilepath + '/' + jsonValue["Light"].asString();

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
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Skybox>& component)
    {
        std::string skyboxFilepath = _parentFilepath + '/' + jsonValue["Skybox"].asString();

        component->SkydomeTexture = dx12::Texture::LoadFromFile(skyboxFilepath);
    }

    void EntityLoader::LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent)
    {
        std::vector<DirectX::XMFLOAT3> points;
        std::vector<DirectX::XMUINT4> groupIndexes;
        std::vector<DirectX::XMFLOAT4> groupWeights;
        std::vector<DirectX::XMFLOAT3> normals;
        std::vector<DirectX::XMFLOAT4> colors;
        std::vector<DirectX::XMFLOAT2> UVs;
        std::vector<DirectX::XMFLOAT3> tangents;
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
                DirectX::XMFLOAT3 v;
                in >> v.x >> v.y >> v.z;
                points.push_back(v);
            }
            else if (input == "vn")
            {
                DirectX::XMFLOAT3 vn;
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
                DirectX::XMFLOAT3 tangent;
                in >> tangent.x >> tangent.y >> tangent.z;
                tangents.push_back(tangent);
            }
            else if (input == "gi")
            {
                DirectX::XMUINT4 groupIndex;
                in >> groupIndex.x >> groupIndex.y >> groupIndex.z >> groupIndex.w;
                groupIndexes.push_back(groupIndex);
            }
            else if (input == "gw")
            {
                DirectX::XMFLOAT4 groupWeight;
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
                    vertex.Color = { 0.8f, 0.8f, 0.8f, 1.0f };
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
} // namespace Helpers
