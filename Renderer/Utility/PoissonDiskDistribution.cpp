#include "stdafx.h"

#include "PoissonDiskDistribution.h"

#include <random>
#include <cmath>

using namespace DirectX;

namespace
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
}

PoissonDiskDistribution::PoissonDiskDistribution(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float minSpawnRadius, float maxSpawnRadius, size_t spawnAttempts)
{
    Reset(minExtent, maxExtent, minSpawnRadius, maxSpawnRadius, spawnAttempts);
}

XMVECTOR PoissonDiskDistribution::Init()
{
    //float randomX = (dis(gen) * (XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent))) + XMVectorGetX(_minExtent);
    //float randomY = (dis(gen) * (XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent))) + XMVectorGetY(_minExtent);
    //float randomZ = (dis(gen) * (XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent))) + XMVectorGetZ(_minExtent);

    float randomX = 0.0f;
    float randomY = 0.0f;
    float randomZ = 0.0f;

    XMVECTOR startPoint = XMVectorSet(randomX, randomY, randomZ, 1.0f);
    _objectLocations.push_back(startPoint);
    _AddPointToGrid(startPoint, 0);

    return startPoint;
}

void PoissonDiskDistribution::Reset(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float minSpawnRadius, float maxSpawnRadius, size_t spawnAttempts)
{
    _minExtent = minExtent;
    _maxExtent = maxExtent;
    _minSpawnRadius = minSpawnRadius;
    _maxSpawnRadius = maxSpawnRadius;
    _spawnAttempts = spawnAttempts;
    _currentSpawnIndex = 0;

    _gridLengthX = XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent);
    _gridLengthY = XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent);
    _gridLengthZ = XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent);

    _cellSize = _minSpawnRadius / std::sqrtf(2);
    _cellsNumX = static_cast<size_t>(std::ceil((XMVectorGetX(_maxExtent) - XMVectorGetX(_minExtent)) / _cellSize));
    _cellsNumY = static_cast<size_t>(std::ceil((XMVectorGetY(_maxExtent) - XMVectorGetY(_minExtent)) / _cellSize));
    _cellsNumZ = static_cast<size_t>(std::ceil((XMVectorGetZ(_maxExtent) - XMVectorGetZ(_minExtent)) / _cellSize));

    // In the case of a plane, not a volume
    // TODO: bad solution, need something else. "If" for the every coord case... seems to be buggy :P
    if (_cellsNumX == 0) _cellsNumX = 1;
    if (_cellsNumY == 0) _cellsNumY = 1;
    if (_cellsNumZ == 0) _cellsNumZ = 1;

    _InitGrid();
}

void PoissonDiskDistribution::Reset()
{
    _InitGrid();
    _objectLocations.clear();
    _currentSpawnIndex = 0;
}

bool PoissonDiskDistribution::TrySpawnStep()
{
    // In case there is no objects in the "active" list left
    if (_currentSpawnIndex == _objectLocations.size())
    {
        return false;
    }

    for (size_t spawnAttempt = 0; spawnAttempt < _spawnAttempts; ++spawnAttempt)
    {
        XMVECTOR newPoint = _GenerateRandomLocationAroundPoint();

        if (!_IsPointInExtents(newPoint) || _CheckCollisions(newPoint))
        {
            continue;
        }

        size_t lastIndex = _objectLocations.size();
        if (!_AddPointToGrid(newPoint, static_cast<int>(lastIndex)))
        {
            continue;
        }

        _objectLocations.push_back(newPoint);
    }
    ++_currentSpawnIndex;

    return true;
}

const std::vector<DirectX::XMVECTOR>& PoissonDiskDistribution::GetLocationsArray() const
{
    return _objectLocations;
}

DirectX::XMVECTOR PoissonDiskDistribution::_GenerateRandomLocationAroundPoint() const
{
    XMVECTOR newLocation = _objectLocations[_currentSpawnIndex];

    float distance = (dis(gen) * (_maxSpawnRadius - _minSpawnRadius)) + _minSpawnRadius;    // Spawn in [r; R]
    float azimuth = dis(gen) * XM_2PI;                                                      // Random azimuth angle [0; 2*PI]
    float polar = dis(gen) * XM_PI;                                                         // Random polar angle [0; PI]

    float x = distance * std::sinf(polar) * std::cosf(azimuth);
    float y = distance * std::sinf(polar) * std::sinf(azimuth);
    float z = distance * std::cosf(polar);

    // In the case of a plane, not a volume
    // TODO: bad solution, need something else. "If" for every coord case... seems to be buggy :P
    if (_gridLengthX < FLT_EPSILON) x = XMVectorGetX(_minExtent);
    if (_gridLengthY < FLT_EPSILON) y = XMVectorGetY(_minExtent);
    if (_gridLengthZ < FLT_EPSILON) z = XMVectorGetZ(_minExtent);

    return XMVectorAdd(XMVectorSet(x, y, z, 1.0f), newLocation);
}

bool PoissonDiskDistribution::_CheckCollisions(const DirectX::XMVECTOR& point) const
{
    int pointIndexX = static_cast<int>((XMVectorGetX(point) + (_gridLengthX * 0.5f)) / _cellSize);
    int pointIndexY = static_cast<int>((XMVectorGetY(point) + (_gridLengthY * 0.5f)) / _cellSize);
    int pointIndexZ = static_cast<int>((XMVectorGetZ(point) + (_gridLengthZ * 0.5f)) / _cellSize);

    for (int x = -2; x <= 2; x++)
    {
        for (int y = -2; y <= 2; ++y)
        {
            for (int z = -2; z <= 2; ++z)
            {
                int indX = pointIndexX + x;
                int indY = pointIndexY + y;
                int indZ = pointIndexZ + z;

                if (indX < 0 || indX >= _cellsNumX)
                    continue;
                if (indY < 0 || indY >= _cellsNumY)
                    continue;
                if (indZ < 0 || indZ >= _cellsNumZ)
                    continue;

                int cubeIndex = _grid[indX][indY][indZ];
                if (cubeIndex == -1)
                    continue;

                float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(_objectLocations[cubeIndex], point)));
                if (distance <= _minSpawnRadius)
                    return true;
            }
        }
    }

    return false;
}

bool PoissonDiskDistribution::_AddPointToGrid(const DirectX::XMVECTOR& point, int index)
{
    int pointIndexX = static_cast<int>((XMVectorGetX(point) + (_gridLengthX * 0.5f)) / _cellSize);
    int pointIndexY = static_cast<int>((XMVectorGetY(point) + (_gridLengthY * 0.5f)) / _cellSize);
    int pointIndexZ = static_cast<int>((XMVectorGetZ(point) + (_gridLengthZ * 0.5f)) / _cellSize);

    if (_grid[pointIndexX][pointIndexY][pointIndexZ] != -1)
        return false;

    _grid[pointIndexX][pointIndexY][pointIndexZ] = (int)index;

    return true;
}

bool PoissonDiskDistribution::_IsPointInExtents(const DirectX::XMVECTOR& point) const
{
    return (XMVectorGetX(point) >= XMVectorGetX(_minExtent) && XMVectorGetY(point) >= XMVectorGetY(_minExtent) && XMVectorGetZ(point) >= XMVectorGetZ(_minExtent)) &&
           (XMVectorGetX(point) <= XMVectorGetX(_maxExtent) && XMVectorGetY(point) <= XMVectorGetY(_maxExtent) && XMVectorGetZ(point) <= XMVectorGetZ(_maxExtent));
}

void PoissonDiskDistribution::_InitGrid()
{
    // Initialize the grid with -1 (empty cells)
    _grid = std::vector<std::vector<std::vector<int>>>(_cellsNumX, 
            std::vector<std::vector<int>>(_cellsNumY, 
            std::vector<int>(_cellsNumZ, -1)));
}
