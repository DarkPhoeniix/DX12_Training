#pragma once

constexpr std::uint32_t InvalidIndex = std::uint32_t(-1);

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

    std::uint32_t InstancesBufferIndex = InvalidIndex;
    std::uint32_t LightsBufferIndex = InvalidIndex;
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
    std::uint32_t LightsBufferIndex = InvalidIndex;
};

struct alignas(16) GPUModelDesc
{
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();

    std::uint32_t AlbedoTextureIndex = InvalidIndex;
    std::uint32_t EmissionTextureIndex = InvalidIndex;
    std::uint32_t NormalMapTextureIndex = InvalidIndex;
    std::uint32_t MetalnessTextureIndex = InvalidIndex;
    std::uint32_t RoughnessTextureIndex = InvalidIndex;

    float EmissionIntensity = 0.0f;
    float MetallicValue = 0.0f;
    float RoughnessValue = 1.0f;
    DirectX::XMVECTOR AlbedoColor = DirectX::XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f);
    DirectX::XMVECTOR EmissionColor = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    std::uint32_t HasMesh = false;
    std::uint32_t BonesBufferIndex = InvalidIndex;
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