#include "RendererPCH.h"

#include "EntityLoader.h"

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

using namespace scene;
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

    void CalculateBoundingVolume(std::shared_ptr<Entity> entity)
    {
        Transformation* transform = entity->GetComponentAs<Transformation>("Transformation");
        Armature* armature = entity->GetComponentAs<Armature>("Armature");
        Mesh* mesh = entity->GetComponentAs<Mesh>("Mesh");

        if (!mesh || !armature)
        {
            return;
        }

        auto UpdateBoneAABB = [](scene::AABBVolume& volume, Bone* bone, XMFLOAT3 position)
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
                    BoneId boneId = mesh->SkinningVertexData[i].BoneIds[j];
                    Bone* bone = armature->GetSortedBones()[boneId];

                    UpdateBoneAABB(volumes[boneId], bone, mesh->VertexData[i].Position);
                }
            }
        }

        for (int i = 0; i < armature->GetBones().size(); ++i)
        {
            Bone* bone = armature->GetSortedBones()[i];

            DirectX::XMVECTOR boxScale = DirectX::XMVectorSubtract(volumes[i].Max, volumes[i].Min) * 0.5f;
            DirectX::XMVECTOR boxLocation = DirectX::XMVectorAdd(volumes[i].Max, volumes[i].Min) * 0.5f;
            DirectX::XMMATRIX invOffset = DirectX::XMMatrixInverse(nullptr, bone->Offset);
            invOffset.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

            bone->AABB.Bounds = DirectX::XMMatrixScalingFromVector(boxScale) * DirectX::XMMatrixTranslationFromVector(boxLocation) * invOffset;
        } 
    }
} // namespace unnamed

namespace scene
{
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
                LoadComponent(jsonRoot, component, entity->GetName());
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

            CalculateBoundingVolume(entity);

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

            // TODO: remove for meshes with armature
            for (const VertexData& vertex : component->VertexData)
            {
                XMVECTOR position = XMLoadFloat3(&vertex.Position);

                component->AABB.Min = XMVectorMin(component->AABB.Min, position);
                component->AABB.Max = XMVectorMax(component->AABB.Max, position);
            }
        }

        void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Light>& component, const std::string& name)
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
            component->OuterAngle = lightData["OuterAngle"].asFloat();
            component->InnerAngle = lightData["InnerAngle"].asFloat();
            component->CastShadows = lightData["CastShadows"].asUInt();

            if (component->CastShadows)
            {
                dx12::ResourceDescription desc = {};
                desc.SetSize({ 1024, 1024 });
                desc.SetDimension(D3D12_RESOURCE_DIMENSION_TEXTURE2D);
                desc.SetFormat(DXGI_FORMAT_D32_FLOAT);

                D3D12_CLEAR_VALUE clearValue;
                clearValue.Format = DXGI_FORMAT_D32_FLOAT;
                clearValue.DepthStencil.Depth = 1;
                clearValue.DepthStencil.Stencil = 0;

                desc.SetClearValue(clearValue);
                desc.SetResourceType(dx12::EResourceType::Texture | dx12::EResourceType::DepthStencil);
                switch (component->Type)
                {
                case LightType::Spot:
                {
                    component->ShadowMaps[0] = std::make_shared<dx12::Resource>();
                    component->ShadowMaps[0]->CreateCommitedResource(desc);
                    component->ShadowMaps[0]->SetName(name + "_ShadowMap");
                }
                break;
                case LightType::Point:
                {
                    for (size_t i = 0; i < 6; ++i)
                    {
                        component->ShadowMaps[i] = std::make_shared<dx12::Resource>();
                        component->ShadowMaps[i]->CreateCommitedResource(desc);
                        component->ShadowMaps[i]->SetName(std::format("{}_ShadowMap_{}", name, i));
                    }
                }
                break;
                }
            }
        }

        void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Skybox>& component)
        {
            std::string skyboxFilepath = _parentFilepath + '/' + jsonValue["Skybox"].asString();

            component->SkydomeTexture = dx12::Texture::LoadFromFile(skyboxFilepath);
        }

        void EntityLoader::LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent)
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
    } // namespace Helpers
} // namespace scene
