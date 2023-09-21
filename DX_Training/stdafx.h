#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>			// For CommandLineToArgvW

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX12 specific headers
#include <d3d12.h>
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#include <DirectXMath.h>        // SIMD-friendly C++ types and functions

#include "d3dx12.h"             // D3D12 extension library

#include <algorithm>
#include <cassert>
#include <string>
#include <chrono>
#include <memory>
#include <map>

#include "Utility/Helpers.h"
#include "Utility/KeyCodes.h"
