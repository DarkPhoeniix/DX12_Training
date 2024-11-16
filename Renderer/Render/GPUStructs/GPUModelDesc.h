#pragma once

struct GPUModelDesc
{
    DirectX::XMMATRIX Transform = DirectX::XMMatrixIdentity();

    UINT AlbedoTextureIndex = -1;
    UINT NormalMapTextureIndex = -1;
    UINT MetalnessTextureIndex = -1;
    UINT RoughnessTextureIndex = -1;
};
