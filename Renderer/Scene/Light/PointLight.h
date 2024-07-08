#pragma once

namespace Core
{
    class GraphicsCommandList;
}

class PointLight
{
public:
    PointLight() = default;
    PointLight(const DirectX::XMVECTOR& position, const DirectX::XMVECTOR& color, float range);
    ~PointLight() = default;

    void SetToShader(Core::GraphicsCommandList& commandList, int index) const;

    void SetPosition(const DirectX::XMVECTOR& position);
    const DirectX::XMVECTOR& GetPosition() const;

    void SetColor(const DirectX::XMVECTOR& color);
    const DirectX::XMVECTOR& GetColor() const;

    void SetRange(float range);
    float GetRange() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

private:
    DirectX::XMVECTOR _position;
    DirectX::XMVECTOR _color;
    float _range;
    float _intensity;
};
