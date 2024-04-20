#pragma once

class FrustumVolume;
class Scene;

class ISceneNode
{
public:
    ISceneNode();
    ISceneNode(const std::string& name, Scene* scene, ISceneNode* parent = nullptr);
    virtual ~ISceneNode();

    DirectX::XMMATRIX GetLocalTransform() const;
    void SetLocalTransform(const DirectX::XMMATRIX& transform);

    DirectX::XMMATRIX GetGlobalTransform() const;

    virtual void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const = 0;
    virtual void DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList) const = 0;

protected:
    friend Scene;

    std::string _name;

    Scene* _scene;
    ISceneNode* _parent;
    std::vector<std::shared_ptr<ISceneNode>> _childNodes;

    DirectX::XMMATRIX _transform;
};
