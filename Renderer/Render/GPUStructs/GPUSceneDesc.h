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
    DirectX::XMFLOAT2 NearFar = { 0, 0 };

    UINT LightsNum = 0;
};
