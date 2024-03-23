#include "stdafx.h"

#include "TextureLoaderDDS.h"
#include "Blob.h"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

namespace TextureLoader
{
    //--------------------------------------------------------------------------------------
    // Macros
    //--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
		((unsigned int)(char)(ch0) | ((unsigned int)(char)(ch1) << 8) |       \
		((unsigned int)(char)(ch2) << 16) | ((unsigned int)(char)(ch3) << 24))
#endif /* defined(MAKEFOURCC) */

#define DDS_MAGIC 0x20534444 // "DDS "

    struct DDS_PIXELFORMAT
    {
        unsigned int size;
        unsigned int flags;
        unsigned int fourCC;
        unsigned int RGBBitCount;
        unsigned int RBitMask;
        unsigned int GBitMask;
        unsigned int BBitMask;
        unsigned int ABitMask;
    };

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC

    struct DDS_HEADER
    {
        unsigned int    size; // must be as 124!!!
        unsigned int    flags;
        unsigned int    height;
        unsigned int    width;
        unsigned int    pitchOrLinearSize;
        unsigned int    depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
        unsigned int    mipMapCount;
        unsigned int    reserved1[11];
        DDS_PIXELFORMAT ddspf;
        unsigned int    caps;
        unsigned int    caps2;
        unsigned int    caps3;
        unsigned int    caps4;
        unsigned int    reserved2;
    };

    struct DDS_HEADER_DXT10
    {
        unsigned int        dxgiFormat;// enum value ( 4 bit )DXGI_FORMAT     dxgiFormat;
        unsigned int        resourceDimension;
        unsigned int        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
        unsigned int        arraySize;
        unsigned int        miscFlags2; // see DDS_MISC_FLAGS2
    };

    // This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
    extern __declspec(selectany) const DDS_PIXELFORMAT DDSPF_DX10 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D', 'X', '1', '0'), 0, 0, 0, 0, 0 };

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE


#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
								   DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
								   DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

#define	D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION	( 2048 )

#define	D3D11_REQ_TEXTURE1D_U_DIMENSION	( 16384 )

#define	D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION	( 2048 )

#define	D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION	( 16384 )

#define	D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION	( 2048 )

#define	D3D11_REQ_TEXTURECUBE_DIMENSION	( 16384 )

#define ISBITMASK( r,g,b,a ) ( header.ddspf.RBitMask == r && header.ddspf.GBitMask == g && header.ddspf.BBitMask == b && header.ddspf.ABitMask == a )

    // Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
    enum DDS_RESOURCE_DIMENSION
    {
        DDS_DIMENSION_TEXTURE1D = 2,
        DDS_DIMENSION_TEXTURE2D = 3,
        DDS_DIMENSION_TEXTURE3D = 4,
    };

    // Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
    enum DDS_RESOURCE_MISC_FLAG
    {
        DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
    };

    //--------------------------------------------------------------------------------------
    // Return the BPP for a particular format
    //--------------------------------------------------------------------------------------
    static size_t BitsPerPixel(_In_ DXGI_FORMAT fmt)
    {
        switch (fmt)
        {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2:
            return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11:
            return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            return 0;
        }
    }
    //--------------------------------------------------------------------------------------
    // Get surface information for a particular format
    //--------------------------------------------------------------------------------------
    static void GetSurfaceInfo(_In_ size_t width,
        _In_ size_t height,
        _In_ DXGI_FORMAT fmt,
        _Out_opt_ size_t* outNumBytes,
        _Out_opt_ size_t* outRowBytes,
        _Out_opt_ size_t* outNumRows)
    {
        size_t numBytes = 0;
        size_t rowBytes = 0;
        size_t numRows = 0;

        bool bc = false;
        bool packed = false;
        bool planar = false;
        size_t bpe = 0;
        switch (fmt)
        {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            packed = true;
            bpe = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            packed = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            planar = true;
            bpe = 4;
            break;
        }

        if (bc)
        {
            size_t numBlocksWide = 0;
            if (width > 0)
            {
                numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
            }
            size_t numBlocksHigh = 0;
            if (height > 0)
            {
                numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
            }
            rowBytes = numBlocksWide * bpe;
            numRows = numBlocksHigh;
            numBytes = rowBytes * numBlocksHigh;
        }
        else if (packed)
        {
            rowBytes = ((width + 1) >> 1) * bpe;
            numRows = height;
            numBytes = rowBytes * height;
        }
        else if (fmt == DXGI_FORMAT_NV11)
        {
            rowBytes = ((width + 3) >> 2) * 4;
            numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
            numBytes = rowBytes * numRows;
        }
        else if (planar)
        {
            rowBytes = ((width + 1) >> 1) * bpe;
            numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
            numRows = height + ((height + 1) >> 1);
        }
        else
        {
            size_t bpp = BitsPerPixel(fmt);
            rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
            numRows = height;
            numBytes = rowBytes * height;
        }

        if (outNumBytes)
        {
            *outNumBytes = numBytes;
        }
        if (outRowBytes)
        {
            *outRowBytes = rowBytes;
        }
        if (outNumRows)
        {
            *outNumRows = numRows;
        }
    }

    static HRESULT FillInitData12(size_t width,
        size_t height,
        size_t depth,
        size_t mipCount,
        size_t arraySize,
        DXGI_FORMAT format,
        size_t maxsize,
        size_t& twidth,
        size_t& theight,
        size_t& tdepth,
        size_t& skipMip
    )
    {
        skipMip = 0;
        twidth = 0;
        theight = 0;
        tdepth = 0;

        size_t NumBytes = 0;
        size_t RowBytes = 0;

        size_t index = 0;
        for (size_t j = 0; j < arraySize; j++)
        {
            size_t w = width;
            size_t h = height;
            size_t d = depth;
            for (size_t i = 0; i < mipCount; i++)
            {
                GetSurfaceInfo(w,
                    h,
                    format,
                    &NumBytes,
                    &RowBytes,
                    nullptr
                );

                if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
                {
                    if (!twidth)
                    {
                        twidth = w;
                        theight = h;
                        tdepth = d;
                    }

                    ++index;
                }
                else if (!j)
                {
                    // Count number of skipped mipmaps (first item only)
                    ++skipMip;
                }

                w = w >> 1;
                h = h >> 1;
                d = d >> 1;
                if (w == 0)
                {
                    w = 1;
                }
                if (h == 0)
                {
                    h = 1;
                }
                if (d == 0)
                {
                    d = 1;
                }
            }
        }

        return (index > 0) ? S_OK : E_FAIL;
    }
    struct stat getStat(const std::string& cFullPath)
    {
        struct stat stat_file;

        memset(&stat_file, 0, sizeof(struct stat));
        stat(cFullPath.c_str(), &stat_file);

        return stat_file;
    }

    size_t getSize(const std::string& cFullPath)
    {
        struct stat stat_file = getStat(cFullPath);
        return static_cast<size_t>(stat_file.st_size);
    }

    void _LoadDDS(const std::string& pFullPath, Resource*& pResource, Base::Blob& memGPU)
    {
        ComPtr<ID3D12Device> device = Application::get().getDevice();

        unsigned int sizeFile = getSize(pFullPath);

        //IO::IFileStream reader;
        FILE* reader = fopen(pFullPath.c_str(), "rb");
        //reader.Open( pFullPath, IO::EMode::in_binary );

        // first read "magic value", mush be as 0x20534444
        unsigned int uiMagic = 0;
        fread(&uiMagic, sizeof(uiMagic), 1, reader);
        

        if (uiMagic != DDS_MAGIC)
        {
            return;
        }

        // next need read "header"
        DDS_HEADER header = {};
        //reader >> header;
        {
            //reader >> header.size; // must be as 124!!!
            //reader >> header.flags;
            //reader >> header.height;
            //reader >> header.width;
            //reader >> header.pitchOrLinearSize;
            //reader >> header.depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
            //reader >> header.mipMapCount;
            //for (int i = 0; i < 11; ++i)
            //    reader >> header.reserved1[i];
            ////reader >> header.reserved1[11];
            //reader >> header.ddspf.size;
            //reader >> header.ddspf.flags;
            //reader >> header.ddspf.fourCC;
            //reader >> header.ddspf.RGBBitCount;
            //reader >> header.ddspf.RBitMask;
            //reader >> header.ddspf.GBitMask;
            //reader >> header.ddspf.BBitMask;
            //reader >> header.ddspf.ABitMask;
            //reader >> header.caps;
            //reader >> header.caps2;
            //reader >> header.caps3;
            //reader >> header.caps4;
            //reader >> header.reserved2;

            fread(&header, sizeof(header), 1, reader);
        }

        // Verify header to validate DDS file
        if (header.size != sizeof(DDS_HEADER) || header.ddspf.size != sizeof(DDS_PIXELFORMAT))
        {
            return;
        }


        UINT width = header.width;
        UINT height = header.height;
        UINT depth = header.depth;

        ETextureType resDim = ETextureType::None;
        unsigned int arraySize = 1;

        size_t mipCount = header.mipMapCount;
        if (0 == mipCount)
        {
            mipCount = 1;
        }

        DXGI_FORMAT resourceFormat = DXGI_FORMAT_UNKNOWN;

        // Check for DX10 extension
        int offsetBits = sizeof(unsigned int) + sizeof(DDS_HEADER);
        DDS_HEADER_DXT10 dx10Header;
        memset(&dx10Header, NULL, sizeof(DDS_HEADER_DXT10));
        if ((header.ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header.ddspf.fourCC))
        {
            offsetBits += sizeof(DDS_HEADER_DXT10);
            //reader >> dx10Header;
            {
                fread(&dx10Header, sizeof(dx10Header), 1, reader);
                //reader >> dx10Header.dxgiFormat;// enum value ( 4 bit )DXGI_FORMAT     dxgiFormat;
                //reader >> dx10Header.resourceDimension;
                //reader >> dx10Header.miscFlag; // see D3D11_RESOURCE_MISC_FLAG
                //reader >> dx10Header.arraySize;
                //reader >> dx10Header.miscFlags2;
            }
            //formatD12 = Driver::ResourceConverter::ConvertTo( (DXGI_FORMAT)dx10Header.dxgiFormat );
            resourceFormat = (DXGI_FORMAT)dx10Header.dxgiFormat;
            arraySize = dx10Header.arraySize;

            switch (dx10Header.resourceDimension)
            {
            case DDS_DIMENSION_TEXTURE1D:

                // D3DX writes 1D textures with a fixed Height of 1
                if ((header.flags & DDS_HEIGHT) && header.height != 1)
                {
                    return;// HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
                }

                depth = 1;
                resDim = ETextureType::Sampler1D;

                //metadata.width = pHeader->width;
                //metadata.height = 1;
                //metadata.depth = 1;
                //metadata.dimension = TEX_DIMENSION_TEXTURE1D;
                break;

            case DDS_DIMENSION_TEXTURE2D:
                if (dx10Header.miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE)
                {
                    depth = 1;
                    arraySize = 6;
                    resDim = ETextureType::SamplerCube;
                }
                else
                {
                    depth = 1;
                    resDim = ETextureType::Sampler2D;
                }
                //metadata.width = pHeader->width;
                //metadata.height = pHeader->height;
                //metadata.depth = 1;
                //metadata.dimension = TEX_DIMENSION_TEXTURE2D;
                break;

            case DDS_DIMENSION_TEXTURE3D:
                if (!(header.flags & DDS_HEADER_FLAGS_VOLUME))
                {
                    return;// HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
                }

                depth = header.depth;
                resDim = ETextureType::Sampler3D;

                //metadata.width = pHeader->width;
                //metadata.height = pHeader->height;
                //metadata.depth = pHeader->depth;
                //metadata.dimension = TEX_DIMENSION_TEXTURE3D;
                break;

            default:
                return;// HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
            }

        }
        else
        {

            if (header.ddspf.flags & DDS_RGB)
            {
                // Note that sRGB formats are written using the "DX10" extended header
                switch (header.ddspf.RGBBitCount)
                {
                case 32:
                    if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
                    {
                        resourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
                        //formatD12 = Driver::EResourceFormat::R8G8B8A8U;
                    }

                    if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
                    {
                        resourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
                        //formatD12 = Driver::EResourceFormat::B8G8R8A8U;
                    }

                    if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
                    {
                        int dd = 23;
                        //Assert( false, "sd" );
                        //return DXGI_FORMAT_B8G8R8X8_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

                    // Note that many common DDS reader/writers (including D3DX) swap the
                    // the RED/BLUE masks for 10:10:10:2 formats. We assumme
                    // below that the 'backwards' header mask is being used since it is most
                    // likely written by D3DX. The more robust solution is to use the 'DX10'
                    // header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

                    // For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
                    if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
                    {
                        resourceFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
                        //formatD12 = Driver::EResourceFormat::R10G10B10A2U;
                    }

                    // No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

                    if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
                    {
                        resourceFormat = DXGI_FORMAT_R16G16_UNORM;
                        //formatD12 = Driver::EResourceFormat::R16G16U;
                    }

                    if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        // Only 32-bit color channel format in D3D9 was R32F
                        resourceFormat = DXGI_FORMAT_R32_FLOAT;
                        //formatD12 = Driver::EResourceFormat::R32F;
                        //return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
                    }
                    break;

                case 24:
                    // No 24bpp DXGI formats aka D3DFMT_R8G8B8
                    break;

                case 16:
                    if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
                    {
                        resourceFormat = DXGI_FORMAT_B5G5R5A1_UNORM;
                        //formatD12 = Driver::EResourceFormat::B5G5R5A1U;
                    }
                    if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
                    {
                        resourceFormat = DXGI_FORMAT_B5G6R5_UNORM;
                        //formatD12 = Driver::EResourceFormat::B5G6R5U;
                    }

                    // No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

                    if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
                    {
                        resourceFormat = DXGI_FORMAT_B4G4R4A4_UNORM;
                        //return DXGI_FORMAT_B4G4R4A4_UNORM;
                    }

                    // No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

                    // No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
                    break;
                }
            }
            else if (header.ddspf.flags & DDS_LUMINANCE)
            {
                if (header.ddspf.RGBBitCount = 8)
                {
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        //formatD12 = Driver::EResourceFormat::R8U;
                        resourceFormat = DXGI_FORMAT_R8_UNORM;
                        //return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }

                    // No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
                }

                if (header.ddspf.RGBBitCount == 16)
                {
                    if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
                    {
                        resourceFormat = DXGI_FORMAT_R16_UNORM;
                        //formatD12 = Driver::EResourceFormat::R16U;
                        //return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                    if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
                    {
                        resourceFormat = DXGI_FORMAT_R8G8_UNORM;
                        //formatD12 = Driver::EResourceFormat::R8G8U;
                        //return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
                    }
                }
            }
            else if (header.ddspf.flags & DDS_ALPHA)
            {
                if (header.ddspf.RGBBitCount == 8)
                {
                    resourceFormat = DXGI_FORMAT_A8_UNORM;
                    //formatD12 = Driver::EResourceFormat::A8U;
                }
            }

            else if ((header.ddspf.flags & DDS_FOURCC))
            {
                if (header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '2') ||
                    header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '3'))
                {
                    resourceFormat = DXGI_FORMAT_BC2_UNORM;
                    //formatD12 = Driver::EResourceFormat::DXT2;
                }
                else if (header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '4') ||
                    header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '5'))
                {
                    resourceFormat = DXGI_FORMAT_BC2_UNORM;
                    //formatD12 = Driver::EResourceFormat::DXT3;
                }
                else if (header.ddspf.fourCC == MAKEFOURCC('B', 'C', '4', 'U') ||
                    MAKEFOURCC('A', 'T', 'I', '1') == header.ddspf.fourCC ||
                    MAKEFOURCC('B', 'C', '4', 'S') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_BC4_UNORM;
                    //formatD12 = Driver::EResourceFormat::DXT4;
                }
                else if (MAKEFOURCC('B', 'C', '4', 'S') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_BC4_SNORM;
                    //formatD12 = Driver::EResourceFormat::DXT4S;
                }
                else if (header.ddspf.fourCC == MAKEFOURCC('B', 'C', '5', 'U') ||
                    MAKEFOURCC('A', 'T', 'I', '2') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_BC4_UNORM;
                    //formatD12 = Driver::EResourceFormat::DXT5;
                }
                else if (MAKEFOURCC('B', 'C', '5', 'S') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_BC4_SNORM;
                    //formatD12 = Driver::EResourceFormat::DXT4S;
                }
                // BC6H and BC7 are written using the "DX10" extended header

                if (MAKEFOURCC('R', 'G', 'B', 'G') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_R8G8_B8G8_UNORM;
                }
                if (MAKEFOURCC('G', 'R', 'G', 'B') == header.ddspf.fourCC)
                {
                    resourceFormat = DXGI_FORMAT_G8R8_G8B8_UNORM;
                }

                if (MAKEFOURCC('Y', 'U', 'Y', '2') == header.ddspf.fourCC)
                {
                    //formatD12 = Driver::EResourceFormat::YUY2;
                    resourceFormat = DXGI_FORMAT_YUY2;
                }

                // Check for D3DFORMAT enums being set here
                switch (header.ddspf.fourCC)
                {
                case 36: // D3DFMT_A16B16G16R16
                    resourceFormat = DXGI_FORMAT_R16G16B16A16_UNORM;
                    //formatD12 = Driver::EResourceFormat::R16G16B16A16U;
                    break;
                case 110: // D3DFMT_Q16W16V16U16
                    //formatD12 = Driver::EResourceFormat::R16G16B16A16S;
                    resourceFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
                    break;
                case 111: // D3DFMT_R16F
                    //formatD12 = Driver::EResourceFormat::R16F;
                    resourceFormat = DXGI_FORMAT_R16_FLOAT;
                    break;
                case 112: // D3DFMT_G16R16F
                    resourceFormat = DXGI_FORMAT_R16G16_FLOAT;
                    //formatD12 = Driver::EResourceFormat::R16G16F;
                    break;
                case 113: // D3DFMT_A16B16G16R16F
                    resourceFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    //formatD12 = Driver::EResourceFormat::R16G16B16A16F;
                    break;
                case 114: // D3DFMT_R32F
                    resourceFormat = DXGI_FORMAT_R32_FLOAT;
                    //formatD12 = Driver::EResourceFormat::R32F;
                    break;
                case 115: // D3DFMT_G32R32F
                    resourceFormat = DXGI_FORMAT_R32G32_FLOAT;
                    //formatD12 = Driver::EResourceFormat::R32G32F;
                    break;
                case 116: // D3DFMT_A32B32G32R32F
                    //formatD12 = Driver::EResourceFormat::R32G32B32A32F;
                    resourceFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    break;
                }
            }
            else
            {
                return;
            }




            if (header.flags & DDS_HEADER_FLAGS_VOLUME)
            {
                resDim = ETextureType::Sampler3D;
            }
            else
            {
                if (header.caps2 & DDS_CUBEMAP)
                {
                    // We require all six faces to be defined
                    if ((header.caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
                    {
                        return;
                    }

                    arraySize = 6;
                    resDim = ETextureType::SamplerCube;
                }
                else
                {
                    resDim = ETextureType::Sampler2D;
                }

                depth = 1;
            }



            switch (resDim)
            {
            case ETextureType::Sampler1D:// D3D11_RESOURCE_DIMENSION_TEXTURE1D:
                if ((arraySize > D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
                    (header.width > D3D11_REQ_TEXTURE1D_U_DIMENSION))
                {
                    return;//  HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
                }
                break;

            case ETextureType::Sampler2D:// D3D11_RESOURCE_DIMENSION_TEXTURE2D:
                if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (header.width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
                    (header.height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION))
                {
                    return;// HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
                }
                break;
            case ETextureType::SamplerCube:
                // This is the right bound because we set arraySize to (NumCubes*6) above
                if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
                    (header.width > D3D11_REQ_TEXTURECUBE_DIMENSION) ||
                    (header.height > D3D11_REQ_TEXTURECUBE_DIMENSION))
                {
                    return;// HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
                }
                break;
            case ETextureType::Sampler3D://D3D11_RESOURCE_DIMENSION_TEXTURE3D:
                if ((arraySize > 1) ||
                    (header.width > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                    (header.height > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
                    (header.depth > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
                {
                    return;// HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
                }
                break;

            default:
                return; //HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
            }

        }

        //Memory::MemData memGPU;

        size_t size_data = sizeFile - offsetBits;

        memGPU.allocate( size_data );

        fread(memGPU.get(), sizeof(char), size_data, reader);
        //reader.read((char*)memGPU.get(), size_data);
        //reader.Read( memGPU.get(), size_data );

        size_t skipMip = 0;
        size_t twidth = 0;
        size_t theight = 0;
        size_t tdepth = 0;

        HRESULT hr = FillInitData12(
            width, height, depth, mipCount, arraySize, resourceFormat, 4096,
            twidth, theight, tdepth, skipMip);



        ResourceDescription desc;

        desc.SetFormat(resourceFormat);
        desc.SetResourceType(EResourceType::Texture);
        desc.SetSize({ (unsigned int)twidth, (unsigned int)theight });
        //desc.SetMipMapCount( static_cast< unsigned int >( mipCount - skipMip ) );
        //desc.SetArraySize( static_cast< unsigned int >( ( tdepth > 1 ) ? tdepth : arraySize ) );


        pResource = new Resource(device, desc);
        //pResource->SetSamperType(resDim);

        pResource->CreateCommitedResource();
        pResource->SetName("Texture_" + pFullPath);

        fclose(reader);

        // add a task for uploading png data into GPU
        //Locator::SysResources.Upload( pResource, std::move( memGPU ) );
   
    }

}


