#pragma once

#include "Scene/Volumes/IVolume.h"

class AABBVolume;

class FrustumVolume : public IVolume
{
public:
    void BuildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix);

    friend bool Intersect(const FrustumVolume& frustum, const AABBVolume& aabb);

    DirectX::XMVECTOR* leftPlane;
    DirectX::XMVECTOR* rightPlane;
    DirectX::XMVECTOR* bottomPlane;
    DirectX::XMVECTOR* topPlane;
    DirectX::XMVECTOR* nearPlane;
    DirectX::XMVECTOR* farPlane;

    DirectX::XMVECTOR planes[6];
};
