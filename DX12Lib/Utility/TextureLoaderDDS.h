#pragma once

#include "Utility/Blob.h"

class Resource;

namespace TextureLoader
{
    void _LoadDDS(const std::string& pFullPath, Resource*& pResource, Base::Blob& memGPU );
}