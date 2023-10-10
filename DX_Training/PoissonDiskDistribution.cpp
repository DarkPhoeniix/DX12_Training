#include "stdafx.h"

#include "PoissonDiskDistribution.h"

#include <random>
#include <cmath>

using namespace DirectX;

namespace
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0f, 1.0f);
}

PoissonDiskDistribution::PoissonDiskDistribution(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float spawnRadius, size_t spawnAttempts)
    : _minExtent(minExtent), _maxExtent(maxExtent), _spawnRadius(spawnRadius), _spawnAttempts(spawnAttempts)
{
    _gridLengthX = XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent);
    _gridLengthY = XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent);
    _gridLengthZ = XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent);

    _cellSize = _spawnRadius / std::sqrt(2);
    _cellsNumX = std::ceil((XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent)) / _cellSize);
    _cellsNumY = std::ceil((XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent)) / _cellSize);

    _grid = std::vector<std::vector<std::vector<int>>>(_cellsNumX, std::vector<std::vector<int>>(_cellsNumY, std::vector<int>(_cellsNumZ, -1)));
}

XMVECTOR PoissonDiskDistribution::Init()
{
    float randomX = (dis(gen) * (XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent))) + XMVectorGetX(_minExtent);
    float randomY = (dis(gen) * (XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent))) + XMVectorGetY(_minExtent);
    float randomZ = (dis(gen) * (XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent))) + XMVectorGetZ(_minExtent);

    XMVECTOR startPoint = XMVectorSet(randomX, randomY, randomZ, 1.0f);
    AddPointToGrid(startPoint, 0);

    return startPoint;
}

void PoissonDiskDistribution::Reset(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float spawnRadius, size_t spawnAttempts)
{
    _minExtent = minExtent;
    _maxExtent = maxExtent;
    _spawnRadius = spawnRadius;
    _spawnAttempts = spawnAttempts;

    _gridLengthX = XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent);
    _gridLengthY = XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent);
    _gridLengthZ = XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent);

    _cellSize = _spawnRadius / std::sqrt(2);
    _cellsNumX = std::ceil((XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent)) / _cellSize);
    _cellsNumY = std::ceil((XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent)) / _cellSize);
    _cellsNumZ = std::ceil((XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent)) / _cellSize);

    _grid = std::vector<std::vector<std::vector<int>>>(_cellsNumX, std::vector<std::vector<int>>(_cellsNumY, std::vector<int>(_cellsNumZ, -1)));
}

bool PoissonDiskDistribution::GetNextLocation(DirectX::XMVECTOR& location)
{


    return false;
}

DirectX::XMVECTOR PoissonDiskDistribution::GenerateRandomLocationAroundPoint() const
{
    XMVECTOR newLocation = _objectLocations[_currentSpawnIndex];

    float distance = (dis(gen) + 1.0f) * _spawnRadius;  // Spawn in [R; 2*R]
    float azimuth = dis(gen) * XM_2PI;                  // Random azimuth angle [0; 2*PI]
    float polar = dis(gen) * XM_PI;                     // Random polar angle [0; PI]

    return XMVectorSet(randomX, randomY, randomZ, 1.0f);
}

bool PoissonDiskDistribution::AddPointToGrid(const DirectX::XMVECTOR& point, size_t index)
{
    int pointIndexX = ((XMVectorGetX(point) + (_gridLengthX * 0.5f)) / _cellSize);
    int pointIndexY = ((XMVectorGetY(point) + (_gridLengthY * 0.5f)) / _cellSize);
    int pointIndexZ = ((XMVectorGetZ(point) + (_gridLengthZ * 0.5f)) / _cellSize);

    if (_grid[pointIndexX][pointIndexY][pointIndexZ] != -1)
        return false;

    _grid[pointIndexX][pointIndexY][pointIndexZ] = index;

    return true;
}
