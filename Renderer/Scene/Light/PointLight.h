#pragma once

#include "ILight.h"

class PointLight : public ILight
{
public:
    PointLight(Scene* scene, ISceneNode* parent = nullptr);
    ~PointLight() = default;

    void SetColor(const DirectX::XMVECTOR& color);
    const DirectX::XMVECTOR& GetColor() const;

    void SetRange(float range);
    float GetRange() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    // ISceneNode
    void LoadNode(const std::string& filepath, Core::GraphicsCommandList& commandList) override;
};
