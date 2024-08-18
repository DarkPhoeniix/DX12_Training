#pragma once

#include "Scene/Volumes/IVolume.h"

class AABBVolume : public IVolume
{
public:
    DirectX::XMVECTOR min;
    DirectX::XMVECTOR max;
};
