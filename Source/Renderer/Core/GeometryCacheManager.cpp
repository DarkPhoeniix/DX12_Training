#include "RendererPCH.h"

#include "GeometryCacheManager.h"

std::unique_ptr<GeometryCacheManager> GeometryCacheManager::_instance = nullptr;

void GeometryCacheManager::Create()
{
    if (!_instance)
        {
        _instance = std::unique_ptr<GeometryCacheManager>(new GeometryCacheManager);
    }
    else
    {
        ERROR("GeometryCacheManager already created!");
    }
}

void GeometryCacheManager::Destroy()
{
    if (_instance)
    {
        _instance.reset();
    }
    else
    {
        ERROR("GeometryCacheManager hasn't been created!");
    }
}

GeometryCacheManager& GeometryCacheManager::Get()
{
    ASSERT(_instance, "GeometryCacheManager not created yet!");
    if (_instance)
    {
        return *_instance;
    }
}

void GeometryCacheManager::Clear()
{
    _cachedGeometries.clear();
}

GeometryHandle GeometryCacheManager::CacheGeometry(std::shared_ptr<rhi::Buffer> geometry)
{
    GeometryHandle handle = static_cast<GeometryHandle>(_cachedGeometries.size());
    _cachedGeometries[handle] = geometry;
    return handle;
}

std::shared_ptr<rhi::Buffer> GeometryCacheManager::GetGeometry(GeometryHandle handle) const
{
    auto it = _cachedGeometries.find(handle);
    if (it == _cachedGeometries.end())
    {
        return nullptr;
    }

    return it->second;
}
