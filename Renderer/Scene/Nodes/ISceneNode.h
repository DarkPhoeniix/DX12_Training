#pragma once

namespace Core
{
    class GraphicsCommandList;
} // namespace Core

namespace SceneLayer
{
    class SceneCache;

    class ISceneNode
    {
    public:
        ISceneNode();
        ISceneNode(SceneCache* cache, ISceneNode* parent = nullptr);
        virtual ~ISceneNode();

        void SetName(const std::string& name);
        const std::string& GetName() const;

        DirectX::XMMATRIX GetGlobalTransform() const;

        DirectX::XMMATRIX GetLocalTransform() const;
        void SetLocalTransform(const DirectX::XMMATRIX& transform);

        void SetParent(ISceneNode* parent);
        ISceneNode* GetParent() const;

        void SetSceneCache(SceneCache* cache);
        SceneCache* GetSceneCache() const;

        virtual void Draw(Core::GraphicsCommandList& commandList) const;
        virtual void DrawAABB(Core::GraphicsCommandList& commandList) const;

        virtual void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList);

    protected:
        std::string _name;

        SceneCache* _sceneCache;
        ISceneNode* _parent;
        std::vector<std::shared_ptr<ISceneNode>> _childNodes;

        DirectX::XMMATRIX _transform;
    };
} // namespace SceneLayer
