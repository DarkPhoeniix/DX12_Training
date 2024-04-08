#pragma once

class FrustumVolume;
class DescriptorHeap;
class Heap;

class ISceneNode
{
public:
    virtual ~ISceneNode() = default;

    DirectX::XMMATRIX GetLocalTransform() const;
    void SetLocalTransform(const DirectX::XMMATRIX& transform);

    DirectX::XMMATRIX GetGlobalTransform() const;

    virtual void Draw(ComPtr<ID3D12GraphicsCommandList> commandList, const FrustumVolume& frustum) const = 0;
    virtual void DrawAABB(ComPtr<ID3D12GraphicsCommandList> commandList) const = 0;

    virtual void UploadTextures(ComPtr<ID3D12GraphicsCommandList> commandList, Heap& heap, DescriptorHeap& descriptorHeap) = 0;

protected:
    std::string _name;

    ISceneNode* _parent = nullptr;
    std::vector<std::shared_ptr<ISceneNode>> _childNodes = {};

    DirectX::XMMATRIX _transform;
};
