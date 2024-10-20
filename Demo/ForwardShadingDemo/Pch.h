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
#include <d3d12.h>
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#include <DirectXMath.h>        // SIMD-friendly C++ types and functions

#include "d3dx12.h"             // D3D12 extension library

#include <json/json.h>

#include <filesystem>
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <map>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include "Utility/KeyCodes.h"
#include "Utility/Defines.h"
#include "Utility/Helpers.h"
#include "Utility/Logger.h"

#include "DXObjects/Device.h"
#include "DXObjects/Resource.h"
