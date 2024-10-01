#pragma once

namespace Core
{
    class GraphicsCommandList;
} // namespace Core

namespace SceneLayer
{
    class FrustumVolume;
    class Scene;

    class ISceneNode
    {
    public:
        ISceneNode();
        ISceneNode(Scene* scene, ISceneNode* parent = nullptr);
        virtual ~ISceneNode();

        void SetName(const std::string& name);
        const std::string& GetName() const;

        DirectX::XMMATRIX GetGlobalTransform() const;

        DirectX::XMMATRIX GetLocalTransform() const;
        void SetLocalTransform(const DirectX::XMMATRIX& transform);

        virtual void Draw(Core::GraphicsCommandList& commandList, const FrustumVolume& frustum) const = 0;
        virtual void DrawAABB(Core::GraphicsCommandList& commandList) const = 0;

        virtual void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList);

    protected:
        friend Scene;

        std::string _name;

        Scene* _scene;
        std::shared_ptr<ISceneNode> _parent;
        std::vector<std::shared_ptr<ISceneNode>> _childNodes;

        DirectX::XMMATRIX _transform;
    };
} // namespace SceneLayer
