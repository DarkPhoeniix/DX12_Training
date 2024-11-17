#pragma once

struct GPULightDesc
{
    DirectX::XMVECTOR direction;
    DirectX::XMVECTOR position;
    DirectX::XMVECTOR color;

    float intensity;
    float range;

    uint32_t type;

    uint32_t pad;
};
