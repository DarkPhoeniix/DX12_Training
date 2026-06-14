#pragma once

namespace img
{
    // ImageFormat corresponds to DXGI_FORMAT and covers all formats supported by the image loaders.
    enum class ImageFormat
    {
        Unknown,

        BC1_TYPELESS = 70,
        BC1_UNORM = 71,
        BC1_UNORM_SRGB = 72,
        BC2_TYPELESS = 73,
        BC2_UNORM = 74,
        BC2_UNORM_SRGB = 75,
        BC3_TYPELESS = 76,
        BC3_UNORM = 77,
        BC3_UNORM_SRGB = 78,
        BC4_TYPELESS = 79,
        BC4_UNORM = 80,
        BC4_SNORM = 81,
        BC5_TYPELESS = 82,
        BC5_UNORM = 83,
        BC5_SNORM = 84,
        BC6H_TYPELESS = 94,
        BC6H_UF16 = 95,
        BC6H_SF16 = 96,
        BC7_TYPELESS = 97,
        BC7_UNORM = 98,
        BC7_UNORM_SRGB = 99,

        R32G32B32A32_FLOAT = 2,
        R32G32B32_FLOAT = 6,
        R32G32_FLOAT = 16,
        R32_FLOAT = 41,

        R16G16B16A16_FLOAT = 10,
        R16G16B16A16_UNORM = 11,
        R16G16B16A16_UINT = 12,
        R16G16B16A16_SNORM = 13,
        R16G16_FLOAT = 34,
        R16G16_UNORM = 35,
        R16G16_SNORM = 37,
        R16_FLOAT = 54,
        R16_UNORM = 56,
        R16_SNORM = 58,

        R8G8B8A8_UNORM = 28,
        R8G8B8A8_UNORM_SRGB = 29,
        R8G8B8A8_SNORM = 31,
        R8G8_UNORM = 49,
        R8G8_SNORM = 51,
        R8_UNORM = 61,
    };
} // namespace img
