#pragma once

#include "Application.h"
#include "Render/Resource.h"
#include "Blob.h"

#include <string>

namespace TextureLoader
{
    void _LoadDDS(const std::string& pFullPath, Resource*& pResource, Base::Blob& memGPU );
}