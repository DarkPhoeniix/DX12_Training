#pragma once

#include "DXObjects/Texture.h"
#include "Scene/Volumes/AABBVolume.h"

#define COMPONENT

enum class LightType
{
    Directional,
    Point,
    Spot
};

struct VertexData
{
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT2 UV;
    DirectX::XMFLOAT3 Tangent;
};

struct IComponent
{
    IComponent() = default;
    IComponent(const std::string& name)
        : ComponentName(name)
    {   }

    std::string ComponentName;
};

struct COMPONENT Transformation : public IComponent
{
    Transformation()
        : IComponent("Transformation")
    {   }

    DirectX::XMMATRIX Transform;
};

struct COMPONENT Material : public IComponent
{
    Material()
        : IComponent("Material")
    {   }

    std::shared_ptr<Core::Texture> Albedo;
    std::shared_ptr<Core::Texture> NormalMap;
    std::shared_ptr<Core::Texture> Metalness;
    std::shared_ptr<Core::Texture> Roughness;
};

struct COMPONENT Mesh : public IComponent
{
    Mesh()
        : IComponent("Mesh")
    {   }

    SceneLayer::AABBVolume AABB;

    std::vector<VertexData> VertexData;
    std::vector<UINT> IndexData;

    std::shared_ptr<Core::Resource> VertexBuffer;
    std::shared_ptr<Core::Resource> IndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
};

struct COMPONENT Light : public IComponent
{
    Light()
        : IComponent("Light")
    {   }

    LightType Type;

    DirectX::XMVECTOR Direction;
    DirectX::XMVECTOR Color;

    float Intensity;
    float Range;
};

struct COMPONENT Skybox : public IComponent
{
    Skybox()
        : IComponent("Skybox")
    {   }

    std::shared_ptr<Core::Texture> SkydomeTexture;
};

struct COMPONENT Tag : public IComponent
{
    Tag()
        : IComponent("Tag")
    {   }

    std::string Name;
};
