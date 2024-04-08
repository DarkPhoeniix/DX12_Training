#pragma once

#include "Interfaces/IVolume.h"

class AABBVolume;

class FrustumVolume : public IVolume
{
public:
    void buildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix);

    friend bool intersect(const FrustumVolume& frustum, const AABBVolume& aabb);

    DirectX::XMVECTOR leftPlane;
    DirectX::XMVECTOR rightPlane;
    DirectX::XMVECTOR bottomPlane;
    DirectX::XMVECTOR topPlane;
    DirectX::XMVECTOR nearPlane;
    DirectX::XMVECTOR farPlane;
};
