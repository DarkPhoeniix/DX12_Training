#pragma once

struct alignas(16) GPUFrameDesc
{
    DirectX::XMMATRIX View = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX Projection = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixIdentity();

    DirectX::XMMATRIX InvView = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX InvProjection = DirectX::XMMatrixIdentity();

    DirectX::XMVECTOR EyePosition = DirectX::XMVectorZero();
    DirectX::XMVECTOR EyeDirection = DirectX::XMVectorZero();

    DirectX::XMUINT2  WindowSize = { 0, 0 };
    DirectX::XMFLOAT2 ReciprocalWindowSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 NearFar = { 0.0f, 0.0f };

    std::uint32_t InstancesBufferIndex = -1;
    std::uint32_t LightsBufferIndex = -1;
    std::uint32_t LightsNum = 0;

    float DeltaTime = 0.0f;
};

struct alignas(16) GPUSceneDesc
{
    DirectX::XMMATRIX ViewProjection = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX View = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX Projection = DirectX::XMMatrixIdentity();

    DirectX::XMMATRIX InvView = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX InvProjection = DirectX::XMMatrixIdentity();

    DirectX::XMVECTOR EyePosition = DirectX::XMVectorZero();
    DirectX::XMVECTOR EyeDirection = DirectX::XMVectorZero();

    DirectX::XMUINT2 WindowSize = { 0, 0 };
    DirectX::XMFLOAT2 ReciprocalWindowSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 NearFar = { 0.0f, 0.0f };

    std::uint32_t LightsNum = 0;
    std::uint32_t LightsBufferIndex = -1;
};

struct alignas(16) GPUModelDesc
{
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();

    std::uint32_t AlbedoTextureIndex = -1;
    std::uint32_t NormalMapTextureIndex = -1;
    std::uint32_t MetalnessTextureIndex = -1;
    std::uint32_t RoughnessTextureIndex = -1;

    std::uint32_t HasMesh = false;
    std::uint32_t BonesBufferIndex = -1;
};

struct alignas(16) GPULightDesc
{
    DirectX::XMVECTOR Direction;
    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR Color;

    float Intensity;
    float Range;
    float OuterAngle;
    float InnerAngle;

    std::uint32_t Type;
    std::uint32_t CastShadows = 0;

    float PerspectiveValues[2];
    std::array<DirectX::XMMATRIX, 6> ViewProj;

    std::uint32_t ShadowMapIndex;
};