
#include "ImageLoader.h"

#include "DDS.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace img
{
    namespace
    {
        static constexpr std::uint32_t DDS_MAGIC = 0x20534444; // "DDS "

        static constexpr std::uint32_t DDS_HEADER_SIZE = sizeof(DDS_MAGIC) + sizeof(DDS_HEADER);
        static constexpr std::uint32_t DDS_HEADER_DXT10_SIZE = DDS_HEADER_SIZE + sizeof(DDS_HEADER_DXT10);

        struct DDS
        {
            ImageFormat ImageFormat;
            DDS_PIXELFORMAT PixelFormat;
        };

        // https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide#common-dds-file-resource-formats-and-associated-header-content
        const DDS DDSMap[] =
        {
            { ImageFormat::BC1_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '1'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC2_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '2'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC2_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '3'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '4'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '5'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('A', '2', 'D', '5'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('x', 'G', 'B', 'R'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('x', 'R', 'B', 'G'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('R', 'G', 'x', 'B'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('x', 'G', 'x', 'R'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('G', 'X', 'R', 'B'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('G', 'R', 'X', 'B'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('R', 'X', 'G', 'B'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC3_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'R', 'G', 'X'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC4_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'C', '4', 'U'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC4_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('A', 'T', 'I', '1'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC5_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'C', '5', 'U'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC5_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('A', 'T', 'I', '2'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC5_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('A', '2', 'X', 'Y'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC6H_UF16,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'C', '6', 'H'), 0, 0, 0, 0, 0 } },

            { ImageFormat::BC7_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'C', '7', 'L'), 0, 0, 0, 0, 0 } },
            { ImageFormat::BC7_UNORM,           { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, MAKEFOURCC('B', 'C', '7', '\0'), 0, 0, 0, 0, 0 } },

            { ImageFormat::R32G32B32A32_FLOAT,  { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, 116, 0, 0, 0, 0, 0 } },
            { ImageFormat::R16G16B16A16_FLOAT,  { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, 113, 0, 0, 0, 0, 0 } },
            { ImageFormat::R16G16B16A16_UNORM,  { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, 36, 0, 0, 0, 0, 0 } },
            { ImageFormat::R16G16B16A16_SNORM,  { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, 110, 0, 0, 0, 0, 0 } },
            { ImageFormat::R16G16_FLOAT,        { sizeof(DDS_PIXELFORMAT), DDPF_FOURCC, 112 ,0 ,0 ,0 ,0 ,0 } }
        };

        ImageFormat GetImageFormat(const DDS_HEADER& header, const DDS_PIXELFORMAT& ddpf)
        {
            uint32_t ddpfFlags = ddpf.Flags;
            constexpr size_t MAP_SIZE = sizeof(DDSMap) / sizeof(DDS);
            size_t index = 0;

            for (index = 0; index < MAP_SIZE; ++index)
            {
                const DDS* entry = &DDSMap[index];

                if (ddpfFlags & DDS_FOURCC)
                {
                    // In case of FourCC codes, ignore any other bits in ddpf.flags
                    if (ddpf.FourCC == entry->PixelFormat.FourCC)
                    {
                        break;
                    }
                }
                else if (ddpf.RGBBitCount == entry->PixelFormat.RGBBitCount)
                {
                    if (entry->PixelFormat.Flags & DDPF_ALPHA)
                    {
                        if (ddpf.ABitMask == entry->PixelFormat.ABitMask)
                        {
                            break;
                        }
                    }
                    else if (entry->PixelFormat.Flags & DDPF_LUMINANCE)
                    {
                        if (entry->PixelFormat.Flags & DDPF_ALPHAPIXELS)
                        {
                            if ((ddpf.RBitMask == entry->PixelFormat.RBitMask) && (ddpf.ABitMask == entry->PixelFormat.ABitMask))
                            {
                                break;
                            }
                        }
                        else
                        {
                            if (ddpf.RBitMask == entry->PixelFormat.RBitMask)
                            {
                                break;
                            }
                        }
                    }
                    else if (entry->PixelFormat.Flags & DDPF_ALPHAPIXELS)
                    {
                        // RGBA
                        if ((ddpf.RBitMask == entry->PixelFormat.RBitMask) && (ddpf.GBitMask == entry->PixelFormat.GBitMask) && 
                            (ddpf.BBitMask == entry->PixelFormat.BBitMask) && (ddpf.ABitMask == entry->PixelFormat.ABitMask))
                        {
                            break;
                        }
                    }
                    else
                    {
                        // RGB
                        if ((ddpf.RBitMask == entry->PixelFormat.RBitMask) && (ddpf.GBitMask == entry->PixelFormat.GBitMask) && (ddpf.BBitMask == entry->PixelFormat.BBitMask))
                        {
                            break;
                        }
                    }
                }
            }

            if (index >= MAP_SIZE)
            {
                return ImageFormat::Unknown;
            }

            return DDSMap[index].ImageFormat;
        }

        constexpr std::string_view EXTENSION_DDS = ".dds";
        constexpr std::string_view EXTENSION_PNG = ".png";
        constexpr std::string_view EXTENSION_HDR = ".hdr";

        std::int32_t GetRequestedChannels(std::int32_t channels)
        {
            if (channels == 3) // 3-channel RGB has no direct GPU format equivalent, so we always expand to 4 channels.
                return 4;
            return channels;
        }

        ImageFormat GetPNGFormat(std::int32_t channels, bool is16bit)
        {
            if (is16bit)
            {
                switch (channels)
                {
                case 1:  return ImageFormat::R16_UNORM;
                case 2:  return ImageFormat::R16G16_UNORM;
                default: return ImageFormat::R16G16B16A16_UNORM;
                }
            }
            else
            {
                switch (channels)
                {
                case 1:  return ImageFormat::R8_UNORM;
                case 2:  return ImageFormat::R8G8_UNORM;
                default: return ImageFormat::R8G8B8A8_UNORM;
                }
            }
        }
    } // namespace unnamed

    Metadata DecodeDDSHeader(const void* source, size_t size)
    {
        Metadata metadata = {};

        unsigned long ddsMagic = *reinterpret_cast<const unsigned long*>(source);
        if (ddsMagic != DDS_MAGIC)
        {
            assert(false && "Invalid DDS file");
            return metadata;
        }

        assert(size >= DDS_HEADER_SIZE && "DDS file is too small to contain a valid header");

        const DDS_HEADER* header = reinterpret_cast<const DDS_HEADER*>(static_cast<const std::uint8_t*>(source) + sizeof(DDS_MAGIC));
        if ((header->Size != sizeof(DDS_HEADER)) || (header->PixelFormat.Size != sizeof(DDS_PIXELFORMAT)))
        {
            assert(false && "Invalid DDS header");
            return metadata;
        }

        metadata.MipLevels = header->MipMapCount ? header->MipMapCount : 1;

        if ((header->PixelFormat.Flags & DDS_FOURCC) && (header->PixelFormat.FourCC == MAKEFOURCC('D', 'X', '1', '0')))
        {
            assert(size >= DDS_HEADER_DXT10_SIZE && "DDS file is too small to contain a valid DXT10 header");

            const DDS_HEADER_DXT10* dxt10Header = reinterpret_cast<const DDS_HEADER_DXT10*>(static_cast<const std::uint8_t*>(source) + DDS_HEADER_SIZE);

            metadata.HasDXT10Header = true;
            metadata.Format         = dxt10Header->Format;
            metadata.Width          = header->Width;
            metadata.Height         = header->Height;
            metadata.Depth          = header->Depth;
            metadata.ArraySize      = 1;

            switch (dxt10Header->ResourceDimension)
            {
            case D3D10_RESOURCE_DIMENSION::D3D10_RESOURCE_DIMENSION_TEXTURE1D:
                metadata.Dimension  = TextureDimension::Texture1D;
                metadata.Height     = 1;
                metadata.Depth      = 1;
                break;
            case D3D10_RESOURCE_DIMENSION::D3D10_RESOURCE_DIMENSION_TEXTURE2D:
                metadata.Dimension  = TextureDimension::Texture2D;
                metadata.Depth      = 1;
                break;
            case D3D10_RESOURCE_DIMENSION::D3D10_RESOURCE_DIMENSION_TEXTURE3D:
                metadata.Dimension  = TextureDimension::Texture3D;
                break;
            default:
                assert(false && "Invalid resource dimension in DXT10 header");
                break;
            }
        }
        else
        {
            metadata.HasDXT10Header = false;
            metadata.Width          = header->Width;
            metadata.Height         = header->Height;
            metadata.Depth          = header->Depth;
            metadata.ArraySize      = 1;

            metadata.Format = GetImageFormat(*header, header->PixelFormat);

            if (header->Caps2 & DDSCAPS2_VOLUME)
            {
                metadata.Dimension = TextureDimension::Texture3D;
            }
            else
            {
                if (header->Caps2 & DDSCAPS2_CUBEMAP)
                {
                    metadata.ArraySize = 6;
                    metadata.Dimension = TextureDimension::TextureCube;
                }
                else
                {
                    metadata.Depth = 1;
                    metadata.Dimension = TextureDimension::Texture2D;
                }
            }
        }

        assert((metadata.Format != ImageFormat::Unknown) && "Unsupported DDS format");

        return metadata;
    }

    Metadata LoadMetadataFromFile(const char* filepath)
    {
        const std::filesystem::path path(filepath);
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        Metadata metadata = {};

        if (extension == EXTENSION_DDS)
            metadata = LoadMetadataFromDDS(filepath);
        else if (extension == EXTENSION_PNG)
            metadata = LoadMetadataFromPNG(filepath);
        else if (extension == EXTENSION_HDR)
            metadata = LoadMetadataFromHDR(filepath);
        else
            assert(false && "Unsupported file extension");

        return metadata;
    }

    Image LoadImageFromFile(const char* filepath)
    {
        const std::filesystem::path path(filepath);
        std::string extension = path.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == EXTENSION_DDS)
            return LoadImageFromDDS(filepath);
        else if (extension == EXTENSION_PNG)
            return LoadImageFromPNG(filepath);
        else if (extension == EXTENSION_HDR)
            return LoadImageFromHDR(filepath);
        else
            assert(false && "Unsupported file extension");

        return Image({});
    }

    Metadata LoadMetadataFromDDS(const char* filepath)
    {
        std::ifstream inFile(std::filesystem::path(filepath), std::ios::in | std::ios::binary | std::ios::ate);
        assert(inFile.is_open() && "Failed to open DDS file");

        std::streampos fileLen = inFile.tellg();
        assert(fileLen < UINT32_MAX && "DDS file is too large");

        inFile.seekg(0, std::ios::beg);
        assert(inFile && "Failed to seek to beginning of DDS file");

        const size_t len = fileLen;

        uint8_t header[DDS_HEADER_DXT10_SIZE] = {};
        const auto headerLen = std::min<size_t>(len, DDS_HEADER_DXT10_SIZE);

        inFile.read(reinterpret_cast<char*>(header), headerLen);
        assert(inFile && "Failed to read DDS header");

        return DecodeDDSHeader(header, headerLen);
    }

    Image LoadImageFromDDS(const char* filepath)
    {
        std::ifstream inFile(std::filesystem::path(filepath), std::ios::in | std::ios::binary | std::ios::ate);
        assert(inFile.is_open() && "Failed to open DDS file");

        std::streampos fileLen = inFile.tellg();
        assert(fileLen < UINT32_MAX && "DDS file is too large");

        inFile.seekg(0, std::ios::beg);
        assert(inFile && "Failed to seek to beginning of DDS file");

        const size_t len = fileLen;

        uint8_t header[DDS_HEADER_DXT10_SIZE] = {};
        const auto headerLen = std::min<size_t>(len, DDS_HEADER_DXT10_SIZE);

        inFile.read(reinterpret_cast<char*>(header), headerLen);
        assert(inFile && "Failed to read DDS header");

        Metadata metadata = DecodeDDSHeader(header, headerLen);

        size_t offset = DDS_HEADER_DXT10_SIZE;
        if (!metadata.HasDXT10Header)
        {
            offset = DDS_HEADER_SIZE;
        }

        inFile.seekg(offset, std::ios::beg);

        const size_t remaining = len - offset;
        assert(remaining > 0 && "DDS file does not contain any image data");

        Image image(metadata);

        std::uint8_t* rawData = image.GetData().data();
        inFile.read(reinterpret_cast<char*>(rawData), remaining);
        assert(inFile && "Failed to read DDS image data");

        return image;
    }

    Metadata LoadMetadataFromPNG(const char* filepath)
    {
        assert(std::filesystem::exists(filepath) && "PNG file does not exist");

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;
        const std::int32_t ok = stbi_info(filepath, &width, &height, &channels);
        assert(ok && "Failed to read PNG metadata");

        const bool is16bit = stbi_is_16_bit(filepath);
        const std::int32_t requestedChannels = GetRequestedChannels(channels);

        Metadata metadata   = {};
        metadata.Width      = static_cast<std::uint64_t>(width);
        metadata.Height     = static_cast<std::uint64_t>(height);
        metadata.Depth      = 1;
        metadata.ArraySize  = 1;
        metadata.MipLevels  = 1;
        metadata.Format     = GetPNGFormat(requestedChannels, is16bit);
        metadata.Dimension  = TextureDimension::Texture2D;

        return metadata;
    }

    Image LoadImageFromPNG(const char* filepath)
    {
        assert(std::filesystem::exists(filepath) && "PNG file does not exist");

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;
        const std::int32_t ok = stbi_info(filepath, &width, &height, &channels);
        assert(ok && "Failed to read PNG metadata");

        const bool is16bit = stbi_is_16_bit(filepath);
        const std::int32_t requestedChannels = GetRequestedChannels(channels);

        Metadata metadata           = {};
        metadata.Depth              = 1;
        metadata.ArraySize          = 1;
        metadata.MipLevels          = 1;
        metadata.Dimension          = TextureDimension::Texture2D;

        std::uint64_t bytesPerPixel = 0;
        void* pixels = nullptr;

        if (is16bit)
        {
            std::uint16_t* p = stbi_load_16(filepath, &width, &height, &channels, requestedChannels);
            assert(p && "Failed to load 16-bit PNG file");
            pixels = p;
            bytesPerPixel = sizeof(std::uint16_t) * requestedChannels;
        }
        else
        {
            std::uint8_t* p = stbi_load(filepath, &width, &height, &channels, requestedChannels);
            assert(p && "Failed to load PNG file");
            pixels = p;
            bytesPerPixel = sizeof(std::uint8_t) * requestedChannels;
        }

        metadata.Width  = static_cast<std::uint64_t>(width);
        metadata.Height = static_cast<std::uint64_t>(height);
        metadata.Format = GetPNGFormat(requestedChannels, is16bit);

        Image image(metadata);

        const std::uint64_t dataSize = static_cast<std::uint64_t>(width) * height * bytesPerPixel;
        std::memcpy(image.GetData().data(), pixels, dataSize);

        stbi_image_free(pixels);

        return image;
    }

    Metadata LoadMetadataFromHDR(const char* filepath)
    {
        assert(std::filesystem::exists(filepath) && "HDR file does not exist");
        assert(stbi_is_hdr(filepath) && "File is not a valid HDR image");

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;
        const std::int32_t ok = stbi_info(filepath, &width, &height, &channels);
        assert(ok && "Failed to read HDR metadata");

        Metadata metadata   = {};
        metadata.Width      = static_cast<std::uint64_t>(width);
        metadata.Height     = static_cast<std::uint64_t>(height);
        metadata.Depth      = 1;
        metadata.ArraySize  = 1;
        metadata.MipLevels  = 1;
        metadata.Format     = ImageFormat::R32G32B32_FLOAT;
        metadata.Dimension  = TextureDimension::Texture2D;

        return metadata;
    }

    Image LoadImageFromHDR(const char* filepath)
    {
        assert(std::filesystem::exists(filepath) && "HDR file does not exist");
        assert(stbi_is_hdr(filepath) && "File is not a valid HDR image");

        std::int32_t width = 0;
        std::int32_t height = 0;
        std::int32_t channels = 0;
        float* pixels = stbi_loadf(filepath, &width, &height, &channels, STBI_rgb);
        assert(pixels && "Failed to load HDR file");

        Metadata metadata   = {};
        metadata.Width      = static_cast<std::uint64_t>(width);
        metadata.Height     = static_cast<std::uint64_t>(height);
        metadata.Depth      = 1;
        metadata.ArraySize  = 1;
        metadata.MipLevels  = 1;
        metadata.Format     = ImageFormat::R32G32B32_FLOAT;
        metadata.Dimension  = TextureDimension::Texture2D;

        Image image(metadata);

        const std::uint64_t dataSize = static_cast<std::uint64_t>(width) * height * sizeof(float) * STBI_rgb;
        std::memcpy(image.GetData().data(), pixels, dataSize);

        stbi_image_free(pixels);

        return image;
    }
} // namespace img
