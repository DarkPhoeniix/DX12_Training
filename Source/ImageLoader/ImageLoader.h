#pragma once

#include "Image.h"

namespace img
{
    // Dispatch to the appropriate loader based on file extension (case-insensitive).
    Metadata LoadMetadataFromFile(const char* filepath);
    Image LoadImageFromFile(const char* filepath);

    // DDS loader — supports BC1-BC7, uncompressed, volume, cubemap, and DXT10 extended header.
    Metadata LoadMetadataFromDDS(const char* filepath);
    Image LoadImageFromDDS(const char* filepath);

    // PNG loader — supports 8-bit and 16-bit per channel; 3-channel RGB is expanded to RGBA.
    Metadata LoadMetadataFromPNG(const char* filepath);
    Image LoadImageFromPNG(const char* filepath);

    // HDR loader — loads Radiance RGBE (.hdr) as R32G32B32_FLOAT. No mip levels.
    Metadata LoadMetadataFromHDR(const char* filepath);
    Image LoadImageFromHDR(const char* filepath);
} // namespace img
