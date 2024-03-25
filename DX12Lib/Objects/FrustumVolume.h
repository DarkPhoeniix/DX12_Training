#pragma once

#include "Interfaces/IVolume.h"

class AABBVolume;

struct Plane
{
    DirectX::XMVECTOR normal;
    float distance;
};

class FrustumVolume : public IVolume
{
public:
    void buildFromProjMatrix(const DirectX::XMMATRIX projectionMatrix);

    friend bool intersect(const FrustumVolume& frustum, const AABBVolume& aabb);

    Plane leftPlane;
    Plane rightPlane;
    Plane bottomPlane;
    Plane topPlane;
    Plane nearPlane;
    Plane farPlane;
};
