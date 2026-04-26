#pragma once

using GeometryHandle = std::uint32_t;
constexpr GeometryHandle InvalidGeometryHandle = GeometryHandle(-1);

class GeometryCacheManager
{
public:
    ~GeometryCacheManager() = default;

    GeometryCacheManager(const GeometryCacheManager& other) = delete;
    GeometryCacheManager(GeometryCacheManager&& other) noexcept = default;

    GeometryCacheManager& operator=(const GeometryCacheManager& other) = delete;
    GeometryCacheManager& operator=(GeometryCacheManager&& other) noexcept = default;

    static void Create();
    static void Destroy();
    static GeometryCacheManager& Get();

    void Clear();

    [[nodiscard]] GeometryHandle CacheGeometry(std::shared_ptr<rhi::Buffer> geometry);
    [[nodiscard]] std::shared_ptr<rhi::Buffer> GetGeometry(GeometryHandle handle) const;

private:
    GeometryCacheManager()= default;

    std::unordered_map<GeometryHandle, std::shared_ptr<rhi::Buffer>> _cachedGeometries;

    static std::unique_ptr<GeometryCacheManager> _instance;
};
