// DX12LibPCH.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
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
#endif

#if defined(max)
#undef max
#endif

// Windows Runtime Library. Needed for ComPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX12 specific headers
#include "d3dx12.h"             // D3D12 extension library
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#include <DirectXTex.h>
#include <DirectXMath.h>

#include "Utility/Logger.h"
#include "Utility/Helpers.h"
#include "Resource.h"
#include "Device.h"

#include <memory>
#include <string>
#include <map>

#endif // DX12LIB_PCH_H
