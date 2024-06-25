#pragma once

namespace Core
{
    class GraphicsCommandList;
};

class DirectionalLight
{
public:
    DirectionalLight() = default;
    DirectionalLight(const DirectX::XMVECTOR& direction, const DirectX::XMVECTOR& color);
    ~DirectionalLight() = default;

    void SetToShader(Core::GraphicsCommandList& commandList, int index) const;

    void SetDirection(const DirectX::XMVECTOR& direction);
    const DirectX::XMVECTOR& GetDirection() const;

    void SetColor(const DirectX::XMVECTOR& color);
    const DirectX::XMVECTOR& GetColor() const;

private:
    DirectX::XMVECTOR _direction;
    DirectX::XMVECTOR _color;
};
