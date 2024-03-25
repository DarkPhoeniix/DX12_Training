#pragma once

class PoissonDiskDistribution
{
public:
    PoissonDiskDistribution(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float minSpawnRadius, float maxSpawnRadius, size_t spawnAttempts = 10);
    ~PoissonDiskDistribution() = default;

    DirectX::XMVECTOR Init();
    void Reset(const DirectX::XMVECTOR& minExtent, const DirectX::XMVECTOR& maxExtent, float minSpawnRadius, float maxSpawnRadius, size_t spawnAttempts = 10);
    void Reset();

    bool TrySpawnStep();

    const std::vector<DirectX::XMVECTOR>& GetLocationsArray() const;

private:
    DirectX::XMVECTOR _GenerateRandomLocationAroundPoint() const;
    bool _CheckCollisions(const DirectX::XMVECTOR& point) const;
    bool _AddPointToGrid(const DirectX::XMVECTOR& point, int index);
    bool _IsPointInExtents(const DirectX::XMVECTOR& point) const;
    void _InitGrid();
    
    DirectX::XMVECTOR _minExtent;
    DirectX::XMVECTOR _maxExtent;

    float _minSpawnRadius;
    float _maxSpawnRadius;

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
