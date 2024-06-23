#include "pch.h"
#include "Scene.h"

Scene::Scene()
    : _root(std::make_shared<Node>())
{   }

Scene::~Scene()
{   }

std::string Scene::GetName() const
{
    return _name;
}

std::shared_ptr<Node> Scene::GetRootNode() const
{
    return _root;
}

bool Scene::Parse(FbxScene* fbxScene)
{
    _name = fbxScene->GetName();
    return _root->Parse(fbxScene->GetRootNode());;
}

bool Scene::Save(const std::string& path) const
{
    std::string rootPath = path + _name + ".scene";

    std::ofstream out(rootPath, std::fstream::out | std::ios_base::binary);

    Json::StreamWriterBuilder builder;
    const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

    Json::Value jsonRoot;
    Json::Value nodes(Json::arrayValue);

    jsonRoot["Name"] = _name.c_str();

    for (const auto& node : _root->GetChildren())
    {
        node->Save(path);

        nodes.append((node->GetName() + ".node").c_str());
    }
    jsonRoot["Nodes"] = nodes;

    writer->write(jsonRoot, &out);


    return false;
}
