
#include "Image.h"

#include <assert.h>

namespace img
{
    namespace
    {
        bool IsBlockCompressed(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::BC1_UNORM:
            case ImageFormat::BC1_UNORM_SRGB:
            case ImageFormat::BC2_UNORM:
            case ImageFormat::BC2_UNORM_SRGB:
            case ImageFormat::BC3_UNORM:
            case ImageFormat::BC3_UNORM_SRGB:
            case ImageFormat::BC4_UNORM:
            case ImageFormat::BC5_UNORM:
            case ImageFormat::BC6H_TYPELESS:
            case ImageFormat::BC6H_SF16:
            case ImageFormat::BC6H_UF16:
            case ImageFormat::BC7_UNORM:
            case ImageFormat::BC7_UNORM_SRGB:
                return true;
            default:
                return false;
            }
        }

        std::uint32_t GetBytesPerPixel(ImageFormat format)
        {
            switch (format)
            {
            case ImageFormat::BC1_UNORM:
                return 8;
            case ImageFormat::BC1_UNORM_SRGB:
                return 8;
            case ImageFormat::BC2_UNORM:
                return 16;
            case ImageFormat::BC2_UNORM_SRGB:
                return 16;
            case ImageFormat::BC3_UNORM:
                return 16;
            case ImageFormat::BC3_UNORM_SRGB:
                return 16;
            case ImageFormat::BC4_UNORM:
                return 8;
            case ImageFormat::BC5_UNORM:
                return 16;
            case ImageFormat::BC6H_TYPELESS:
                return 16;
            case ImageFormat::BC6H_SF16:
                return 16;
            case ImageFormat::BC6H_UF16:
                return 16;
            case ImageFormat::BC7_UNORM:
                return 16;
            case ImageFormat::BC7_UNORM_SRGB:
                return 16;
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
            case ImageFormat::R8G8B8A8_UNORM_SRGB:
                return 4;
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
    } // namespace unnamed

    Image::Image(const Metadata& metadata)
        : _metadata(metadata)
    {
        _size = 0;
        _pixelSize = GetBytesPerPixel(metadata.Format);
        _imageCount = metadata.MipLevels;

        const bool isBC = IsBlockCompressed(metadata.Format);

        for (std::uint32_t i = 0; i < _imageCount; ++i)
        {
            ImageSlice slice = {};
            slice.Width = std::max<std::uint32_t>(1, metadata.Width >> i);
            slice.Height = std::max<std::uint32_t>(1, metadata.Height >> i);

            if (isBC)
            {
                const std::uint32_t blockWidth = std::max<std::uint32_t>(1, (slice.Width + 3) / 4);
                const std::uint32_t blockHeight = std::max<std::uint32_t>(1, (slice.Height + 3) / 4);
                slice.RowPitch = blockWidth * _pixelSize;
                slice.SlicePitch = slice.RowPitch * blockHeight;
            }
            else
            {
                slice.RowPitch = slice.Width * _pixelSize;
                slice.SlicePitch = slice.RowPitch * slice.Height;
            }

            _images.push_back(slice);
            _size += slice.SlicePitch;
        }

        _memory.resize(_size);
        PatchPixelPointers();
    }

    Image::Image(Image&& other) noexcept
        : _metadata(other._metadata)
        , _images(std::move(other._images))
        , _memory(std::move(other._memory))
        , _size(other._size)
        , _pixelSize(other._pixelSize)
        , _imageCount(other._imageCount)
    {
        PatchPixelPointers();
    }

    Image& Image::operator=(Image&& other) noexcept
    {
        if (this != &other)
        {
            _metadata   = other._metadata;
            _images     = std::move(other._images);
            _memory     = std::move(other._memory);
            _size       = other._size;
            _pixelSize  = other._pixelSize;
            _imageCount = other._imageCount;

            PatchPixelPointers();
        }

        return *this;
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

    std::uint32_t img::Image::GetSliceCount() const
    {
        return _imageCount;
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

    void Image::PatchPixelPointers()
    {
        std::uint64_t offset = 0;

        for (ImageSlice& slice : _images)
        {
            slice.Pixels = _memory.data() + offset;
            offset += slice.SlicePitch;
        }
    }
} // namespace img
