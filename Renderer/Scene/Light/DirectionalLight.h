#pragma once

#include "ILight.h"

class DirectionalLight : public ILight
{
public:
    DirectionalLight(Scene* scene, ISceneNode* parent);
    ~DirectionalLight() = default;

    void SetDirection(const DirectX::XMVECTOR& direction);
    const DirectX::XMVECTOR& GetDirection() const;

    void SetColor(const DirectX::XMVECTOR& color);
    const DirectX::XMVECTOR& GetColor() const;

    // ISceneNode
    void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;
};
