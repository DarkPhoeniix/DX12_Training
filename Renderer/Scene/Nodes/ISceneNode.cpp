#include "stdafx.h"

#include "ISceneNode.h"

#include "Scene/NodeFactory.h"

using namespace DirectX;

namespace
{
    DirectX::XMMATRIX ParseTransformationMatrix(const Json::Value& transform)
    {
        // TODO: this func is ugly, rework later
        DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();

        int sz = transform["Transform"].size();
        Json::Value val;
        for (int i = 0; i < sz; ++i)
        {
            val = transform["Transform"][std::format("r{}", i).c_str()];
            std::stringstream iss(val.asCString());
            float value = 0;

            DirectX::XMFLOAT4 r;
            iss >> r.x >> r.y >> r.z >> r.w;

            matrix.r[i] = DirectX::XMLoadFloat4(&r);
        }

        return matrix;
    }
} // namespace unnamed

namespace SceneLayer
{
    ISceneNode::ISceneNode()
        : _sceneCache(nullptr)
        , _parent(nullptr)
        , _childNodes{}
        , _transform{}
    {
    }

    ISceneNode::ISceneNode(SceneCache* cache, ISceneNode* parent)
        : _sceneCache(cache)
        , _parent(parent)
        , _childNodes{}
        , _transform(XMMatrixIdentity())
    {
    }

    ISceneNode::~ISceneNode()
    {
        _sceneCache = nullptr;
        _parent = nullptr;
    }

    void ISceneNode::SetName(const std::string& name)
    {
        _name = name;
    }

    const std::string& ISceneNode::GetName() const
    {
        return _name;
    }

    XMMATRIX ISceneNode::GetGlobalTransform() const
    {
        XMMATRIX globalTransform = _transform;
        if (_parent)
        {
            globalTransform *= _parent->GetGlobalTransform();
        }

        return globalTransform;
    }

    XMMATRIX ISceneNode::GetLocalTransform() const
    {
        return _transform;
    }

    void ISceneNode::SetLocalTransform(const XMMATRIX& transform)
    {
        _transform = transform;
    }

    void ISceneNode::SetParent(ISceneNode* parent)
    {
        _parent = parent;
    }

    ISceneNode* ISceneNode::GetParent() const
    {
        return _parent;
    }

    void ISceneNode::SetSceneCache(SceneCache* cache)
    {
        _sceneCache = cache;
    }

    SceneCache* ISceneNode::GetSceneCache() const
    {
        return _sceneCache;
    }

    void ISceneNode::Draw(Core::CommandList& commandList) const
    {    }

    void ISceneNode::DrawAABB(Core::CommandList& commandList) const
    {    }

    void ISceneNode::LoadNode(const std::string& filepath, Core::CommandList& commandList)
    {
        LOG_INFO("Parsing node " + filepath);

        ASSERT(std::filesystem::exists(std::filesystem::path(filepath)), std::format("Failed to parse a node from {}", filepath));

        std::ifstream in(filepath, std::ifstream::in | std::ifstream::binary);

        Json::Value root;
        in >> root;

        // Parse name
        _name = root["Name"].asCString();

        // Parse trsformation matrix
        _transform = ParseTransformationMatrix(root);

        // Parse children nodes
        for (auto& node : root["Nodes"])
        {
            // TODO: redundant file opening...
            std::string nodeFilepath = std::filesystem::path(filepath).parent_path().string() + '/' + node.asCString();
            std::ifstream nodeIn(nodeFilepath, std::ifstream::in | std::ifstream::binary);

            Json::Value nodeRoot;
            nodeIn >> nodeRoot;

            std::shared_ptr<ISceneNode> child = std::shared_ptr<ISceneNode>(NodeFactory::Create<ISceneNode>(nodeRoot["Type"].asCString()));
            child->SetParent(this);
            child->SetSceneCache(_sceneCache);
            child->LoadNode(nodeFilepath, commandList);

            _childNodes.push_back(child);
        }
    }
} // namespace SceneLayer
