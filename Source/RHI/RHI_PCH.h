// RHI_PCH.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef DX12LIB_PCH_H
#define DX12LIB_PCH_H

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>			// For CommandLineToArgvW

// The min/max macros conflict with like-named member functions from <algorithm>
#if defined(min)
#undef min
#endif // defined(min)

#if defined(max)
#undef max
#endif // defined(max)

// Windows Runtime Library. Needed for ComPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

#if _DEBUG
#define ENABLE_DEBUG_NAMES 1
#define ENABLE_DEBUG_DESC 1
#define ENABLE_DEVICE_DEBUG 1
#endif // _DEBUG

// DirectX12 specific headers
#if USE_D3D12
#include <directx/d3dx12.h>     // D3D12 extension library
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#endif // USE_D3D12

#include "Device.h"

#include "Logger/Logger.h"

#include "Utility/Helpers.h"

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>

#endif // DX12LIB_PCH_H
