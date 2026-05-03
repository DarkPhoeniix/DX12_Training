#pragma once

#include "ImageFormat.h"

#include <cstdint>
#include <vector>

namespace img
{
    enum class TextureDimension
    {
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    struct Metadata
    {
        std::uint64_t Width;
        std::uint64_t Height;
        std::uint64_t Depth;
        std::uint64_t ArraySize;
        std::uint64_t MipLevels;
        ImageFormat Format;
        TextureDimension Dimension;
        bool HasDXT10Header;
    };

    struct ImageSlice
    {
        std::uint64_t Width;
        std::uint64_t Height;

        std::uint64_t RowPitch;
        std::uint64_t SlicePitch;

        std::uint8_t* Pixels;
    };

    class Image
    {
    public:
        Image(const Metadata& metadata);
        Image(const Image&) = delete;
        Image(Image&&) = default;
        ~Image() = default;

        Image& operator=(const Image&) = delete;
        Image& operator=(Image&&) = default;

        const Metadata& GetMetadata() const;

        std::uint64_t GetWidth() const;
        std::uint64_t GetHeight() const;
        std::uint32_t GetPixelSize() const;

        const ImageSlice& GetImageSlice(std::uint32_t mipLevel) const;
        const std::vector<ImageSlice>& GetImageSlices() const;

        std::vector<std::uint8_t>& GetData();
        const std::vector<std::uint8_t>& GetData() const;

    protected:
        Metadata _metadata;

        std::vector<ImageSlice> _images;
        std::vector<std::uint8_t> _memory;

        std::uint64_t _size;
        std::uint32_t _pixelSize;
        std::uint32_t _imageCount;
    };
} // namespace img
