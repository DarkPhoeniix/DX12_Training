#pragma once

class PoissonDiskDistribution
{
public:
    PoissonDiskDistribution(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float spawnRadius, size_t spawnAttempts = 10);
    ~PoissonDiskDistribution() = default;

    DirectX::XMVECTOR Init();
    void Reset(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float spawnRadius, size_t spawnAttempts = 10);

    bool GetNextLocation(DirectX::XMVECTOR& location);

private:
    DirectX::XMVECTOR GenerateRandomLocationAroundPoint() const;
    bool AddPointToGrid(const DirectX::XMVECTOR& point, size_t index);
    
    DirectX::XMVECTOR _minExtent;
    DirectX::XMVECTOR _maxExtent;
    float _spawnRadius;
    size_t _spawnAttempts;

    std::vector<DirectX::XMVECTOR> _objectLocations;
    size_t _currentSpawnIndex;

    std::vector<std::vector<std::vector<int>>> _grid;
    float _gridLengthX;
    float _gridLengthY;
    float _gridLengthZ;
    float _cellSize;
    size_t _cellsNumX;
    size_t _cellsNumY;
    size_t _cellsNumZ;
};
