#pragma once

#include "Image.h"

namespace img
{
    Metadata LoadMetadataFromDDS(const char* filepath);
    Image LoadImageFromDDS(const char* filepath);
} // namespace img
