#include "stdafx.h"

#include "EntityLoader.h"

#include "Scene/Entity.h"

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
        for (auto& node : jsonRoot["Nodes"])
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
        if (!jsonRoot["Tag"].isNull())
        {
            std::shared_ptr<Tag> component = std::make_shared<Tag>();
            LoadComponent(jsonRoot, component);
            entity->AddComponent(component);
        }

        return entity;
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Transformation>& component)
    {

        component->Transform = DirectX::XMMatrixIdentity();

        int matrixSize = jsonValue["Transform"].size();
        for (int i = 0; i < matrixSize; ++i)
        {
            component->Transform.r[i] = ParseVector(jsonValue["Transform"][std::format("r{}", i).c_str()].asString());
        }
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

        component->Albedo = Core::Texture::LoadFromFile(albedoFilepath);
        component->NormalMap = Core::Texture::LoadFromFile(normalFilepath);
        component->Metalness = Core::Texture::LoadFromFile(metalnessFilepath);
        component->Roughness = Core::Texture::LoadFromFile(roughnessFilepath);
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

        component->Color = ParseVector(lightData["Color"].asString());
        component->Intensity = lightData["Intensity"].asFloat();
        component->Range = lightData["Range"].asFloat();
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Skybox>& component)
    {
        std::string skyboxFilepath = _parentFilepath + '/' + jsonValue["Skybox"].asString();

        component->SkydomeTexture = Core::Texture::LoadFromFile(skyboxFilepath);
    }

    void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Tag>& component)
    {
        component->Name = jsonValue["Tag"].asString();
    }

    void EntityLoader::LoadRawMesh(const std::string& filepath, const std::shared_ptr<Mesh>& meshComponent)
    {
        std::vector<DirectX::XMFLOAT3> points;
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

                    meshComponent->VertexData.push_back(vertex);
                    meshComponent->IndexData.push_back(index++);
                }
            }
        }
    }
} // namespace Helpers
