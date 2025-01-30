#pragma once

struct GPUSceneDesc
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

    UINT LightsNum = 0;
};

struct GPUModelDesc
{
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();

    UINT AlbedoTextureIndex = -1;
    UINT NormalMapTextureIndex = -1;
    UINT MetalnessTextureIndex = -1;
    UINT RoughnessTextureIndex = -1;

    bool UseSkinning = false;
};

struct GPULightDesc
{
    DirectX::XMVECTOR Direction;
    DirectX::XMVECTOR Position;
    DirectX::XMVECTOR Color;

    float Intensity;
    float Range;
    float OuterAngle;
    float InnerAngle;

    std::array<DirectX::XMMATRIX, 6> ViewProj;

    uint32_t Type;
    bool CastShadows = false;

    std::array<UINT, 6> ShadowMapIndexes;
};