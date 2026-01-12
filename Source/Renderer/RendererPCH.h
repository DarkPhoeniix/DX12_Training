#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>			// For CommandLineToArgvW

// The min/max macros conflict with like-named member functions from <algorithm>
#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

// Windows Runtime Library. Needed for ComPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX12 specific headers
#include <directx/d3dx12.h>     // D3D12 extension library
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#include <DirectXTex.h>
#include <DirectXMath.h>

#include <pix3.h>

#include <json/json.h>

#include <filesystem>
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cstdint>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <map>

#include "Logger/Logger.h"

#include "Helpers/Defines.h"
#include "Utility/Helpers.h"

#include "RHI/ResourceBarrier.h"
#include "RHI/Resource.h"
#include "RHI/ResourceFactory.h"
#include "RHI/CommandList.h"
#include "RHI/Device.h"

#include "RenderGraph/RenderGraphResourceId.h"
