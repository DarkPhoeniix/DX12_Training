
#include "Image.h"

#include <assert.h>

namespace img
{
    namespace
    {
        std::uint32_t GetBytesPerPixel(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::R32G32B32A32_FLOAT:
                return 16;
            case ImageFormat::R32G32B32_FLOAT:
                return 12;
            case ImageFormat::R32G32_FLOAT:
                return 8;
            case ImageFormat::R32_FLOAT:
                return 4;
            case ImageFormat::R16G16B16A16_FLOAT:
                return 8;
            case ImageFormat::R16G16_FLOAT:
                return 4;
            case ImageFormat::R16_FLOAT:
                return 2;
            case ImageFormat::R8G8B8A8_UNORM:
                return 4;
            case ImageFormat::R8G8_UNORM:
                return 2;
            case ImageFormat::R8_UNORM:
                return 1;
            default:
                assert(false && "Unsupported image format");
                return 0;
            }
        }
    }

    Image::Image(const Metadata& metadata)
        : _metadata(metadata)
    {
        _size = 0;
        _pixelSize = GetBytesPerPixel(metadata.Format);
        _imageCount = metadata.MipLevels;

        for (std::uint32_t i = 0; i < _imageCount; ++i)
        {
            ImageSlice slice = {};
            slice.Width = std::max<std::uint32_t>(1, metadata.Width >> i);
            slice.Height = std::max<std::uint32_t>(1, metadata.Height >> i);

            slice.RowPitch = slice.Width * _pixelSize;
            slice.SlicePitch = slice.RowPitch * slice.Height;

            _images.push_back(slice);
            _size += slice.SlicePitch;
        }

        _memory.resize(_size);
    }

    const Metadata& Image::GetMetadata() const
    {
        return _metadata;
    }

    std::uint64_t Image::GetWidth() const
    {
        return _metadata.Width;
    }

    std::uint64_t Image::GetHeight() const
    {
        return _metadata.Height;
    }

    std::uint32_t Image::GetPixelSize() const
    {
        return _pixelSize;
    }

    const ImageSlice& Image::GetImageSlice(std::uint32_t mipLevel) const
    {
        assert(mipLevel < _imageCount && "Invalid mip level");
        return _images[mipLevel];
    }

    const std::vector<ImageSlice>& Image::GetImageSlices() const
    {
        return _images;
    }

    std::vector<std::uint8_t>& Image::GetData()
    {
        return _memory;
    }

    const std::vector<std::uint8_t>& Image::GetData() const
    {
        return _memory;
    }
} // namespace img
