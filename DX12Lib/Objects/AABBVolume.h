#pragma once

#include "Interfaces/IVolume.h"

class AABBVolume : public IVolume
{
public:
    DirectX::XMVECTOR min;
    DirectX::XMVECTOR max;

private:

};
