#pragma once

#include "Image.h"

#define DDS_FOURCC        0x00000004
#define DDS_RGB           0x00000040
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
                  (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch0)) \
                | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch1)) << 8) \
                | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch2)) << 16) \
                | (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch3)) << 24))
#endif // MAKEFOURCC

// DDS Flags to indicate which members contain valid data.
#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

// DDS Caps specifies the complexity of the surfaces stored.
#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_MIPMAP              0x00400000
#define DDSCAPS_TEXTURE             0x00001000

// DDS Caps2 specifies the additional detail about the surfaces stored.
#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

// DDS Pixel Format flags which indicate what type of data is in the surface.
#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_ALPHA                  0x00000002
#define DDPF_FOURCC                 0x00000004
#define DDPF_RGB                    0x00000040
#define DDPF_YUV                    0x00000200
#define DDPF_LUMINANCE              0x00020000

#define DDS_RESOURCE_MISC_TEXTURECUBE 0x00000004

#define DDS_ALPHA_MODE_UNKNOWN      0x00000000
#define DDS_ALPHA_MODE_STRAIGHT     0x00000001
#define DDS_ALPHA_MODE_PREMULTIPLIED 0x00000002
#define DDS_ALPHA_MODE_OPAQUE       0x00000003
#define DDS_ALPHA_MODE_CUSTOM       0x00000004

namespace img
{
    enum class D3D10_RESOURCE_DIMENSION
    {
        D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
        D3D10_RESOURCE_DIMENSION_BUFFER = 1,
        D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
        D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
        D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
    };

    struct DDS_PIXELFORMAT
    {
        std::uint32_t Size;
        std::uint32_t Flags;
        std::uint32_t FourCC;
        std::uint32_t RGBBitCount;
        std::uint32_t RBitMask;
        std::uint32_t GBitMask;
        std::uint32_t BBitMask;
        std::uint32_t ABitMask;
    };

    struct DDS_HEADER
    {
        std::uint32_t   Size;
        std::uint32_t   Flags;
        std::uint32_t   Height;
        std::uint32_t   Width;
        std::uint32_t   PitchOrLinearSize;
        std::uint32_t   Depth;
        std::uint32_t   MipMapCount;
        std::uint32_t   Reserved1[11];
        DDS_PIXELFORMAT PixelFormat;
        std::uint32_t   Caps;
        std::uint32_t   Caps2;
        std::uint32_t   Caps3;
        std::uint32_t   Caps4;
        std::uint32_t   Reserved2;
    };

    struct DDS_HEADER_DXT10
    {
        ImageFormat              Format;
        D3D10_RESOURCE_DIMENSION ResourceDimension;
        std::uint32_t            MiscFlag;
        std::uint32_t            ArraySize;
        std::uint32_t            MiscFlags2;
    };
} // namespace img
