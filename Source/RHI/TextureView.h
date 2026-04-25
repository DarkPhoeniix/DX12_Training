#pragma once

#include "Texture.h"

namespace rhi
{
    class TextureView
    {
    public:
        TextureView(Texture* texture,
            ResourceViewType viewType,
            TextureDimension dimension = TextureDimension::Texture2D,
            std::uint32_t mostDetailedMip = 0,
            std::uint32_t mipLevels = 0,
            std::uint32_t mipSlice = 0,
            std::uint32_t firstArraySlice = 0,
            std::uint32_t arraySize = 0,
            std::uint32_t planeSlice = 0,
            float resourceMinLODClamp = 0.0f)
            : _texture(texture)
            , _viewType(viewType)
            , _dimension(dimension)
            , _mostDetailedMip(mostDetailedMip)
            , _mipLevels(mipLevels)
            , _mipSlice(mipSlice)
            , _firstArraySlice(firstArraySlice)
            , _arraySize(arraySize)
            , _planeSlice(planeSlice)
            , _resourceMinLODClamp(resourceMinLODClamp)
        {
            ASSERT(texture, "TextureView cannot be created with a null texture pointer.");
            if (texture)
            {
                _format = texture->GetFormat();
            }
        }
        TextureView(const TextureView&) = delete;
        TextureView(TextureView&&) noexcept = default;
        virtual ~TextureView() = default;

        TextureView& operator=(const TextureView&) = delete;
        TextureView& operator=(TextureView&&) noexcept = default;

        [[nodiscard]] Texture* GetTexture() const { return _texture; }

        void SetType(ResourceViewType viewType) { _viewType = viewType; }
        [[nodiscard]] ResourceViewType GetType() const { return _viewType; }

        void SetFormat(Format format) { _format = format; }
        [[nodiscard]] Format GetFormat() const { return _format; }

        void SetDimension(TextureDimension dimension) { _dimension = dimension; }
        [[nodiscard]] TextureDimension GetDimension() const { return _dimension; }

        void SetMostDetailedMip(std::uint32_t mostDetailedMip) { _mostDetailedMip = mostDetailedMip; }
        [[nodiscard]] std::uint32_t GetMostDetailedMip() const { return _mostDetailedMip; }

        void SetMipLevels(std::uint32_t mipLevels) { _mipLevels = mipLevels; }
        [[nodiscard]] std::uint32_t GetMipLevels() const { return _mipLevels; }

        void SetMipSlice(std::uint32_t mipSlice) { _mipSlice = mipSlice; }
        [[nodiscard]] std::uint32_t GetMipSlice() const { return _mipSlice; }

        void SetFirstArraySlice(std::uint32_t firstArraySlice) { _firstArraySlice = firstArraySlice; }
        [[nodiscard]] std::uint32_t GetFirstArraySlice() const { return _firstArraySlice; }

        void SetArraySize(std::uint32_t arraySize) { _arraySize = arraySize; }
        [[nodiscard]] std::uint32_t GetArraySize() const { return _arraySize; }

        void SetPlaneSlice(std::uint32_t planeSlice) { _planeSlice = planeSlice; }
        [[nodiscard]] std::uint32_t GetPlaneSlice() const { return _planeSlice; }

        void SetResourceMinLODClamp(float resourceMinLODClamp) { _resourceMinLODClamp = resourceMinLODClamp; }
        [[nodiscard]] float GetResourceMinLODClamp() const { return _resourceMinLODClamp; }

    private:
        Texture* _texture = nullptr;

        ResourceViewType _viewType;

        Format _format = Format::UNKNOWN;
        TextureDimension _dimension;

        std::uint32_t _mostDetailedMip = 0;
        std::uint32_t _mipLevels = 0;
        std::uint32_t _mipSlice = 0;
        std::uint32_t _firstArraySlice = 0;
        std::uint32_t _arraySize = 0;
        std::uint32_t _planeSlice = 0;
        float _resourceMinLODClamp = 0.0f;
    };
} // namespace rhi
