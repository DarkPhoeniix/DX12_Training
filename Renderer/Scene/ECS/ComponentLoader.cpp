#include "stdafx.h"

#include "ComponentLoader.h"

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

    DirectX::XMMATRIX ParseTransformationMatrix(const Json::Value& transform)
    {
        // TODO: this func is ugly, rework later
        DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

        int sz = transform["Transform"].size();
        Json::Value val;
        for (int i = 0; i < sz; ++i)
        {
            matrix.r[i] = ParseVector(transform["Transform"][std::format("r{}", i).c_str()].asString());
        }

        return matrix;
    }
} // namespace unnamed

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
    component->Transformation = ParseTransformationMatrix(jsonValue["Transform"]);
}

void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Material>& component)
{
    std::string materialFilepth = _parentFilepath + '/' + jsonValue["Material"].asString();

    std::ifstream file(materialFilepth, std::ios_base::in | std::ios_base::binary);
    Json::Value materialData;
    file >> materialData;

    std::string albedoFilepath      = _parentFilepath + '/' + materialData["Albedo"].asString();
    std::string normalFilepath      = _parentFilepath + '/' + materialData["Normal"].asString();
    std::string metalnessFilepath   = _parentFilepath + '/' + materialData["Metalness"].asString();
    std::string roughnessFilepath   = _parentFilepath + '/' + materialData["Roughness"].asString();

    component->Albedo               = Core::Texture::LoadFromFile(albedoFilepath);
    component->NormalMap            = Core::Texture::LoadFromFile(albedoFilepath);
    component->Metalness            = Core::Texture::LoadFromFile(albedoFilepath);
    component->Roughness            = Core::Texture::LoadFromFile(albedoFilepath);
}

void EntityLoader::LoadComponent(Json::Value& jsonValue, const std::shared_ptr<Mesh>& component)
{
    std::string meshFilepth = _parentFilepath + '/' + jsonValue["Mesh"].asString();

    std::ifstream file(meshFilepth, std::ios_base::in | std::ios_base::binary);
    Json::Value meshData;
    file >> meshData;

    // TODO: implement Mesh parsing
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
