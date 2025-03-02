
#ifndef RENDER_GRAPH_PCH_H
#define RENDER_GRAPH_PCH_H

// Windows Runtime Library. Needed for ComPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX12 specific headers
#include <d3dx12.h>             // D3D12 extension library
#include <dxgi1_6.h>            // Microsoft DirectX Graphics Infrastructure
#include <d3dcompiler.h>        // Contains functions to compile HLSL code at runtime
#include <DirectXMath.h>        // SIMD-friendly C++ types and functions

#include <cstdint>
#include <memory>
#include <string>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

#include "Utility/Logger.h"
#include "Utility/Helpers.h"
#include "Resource.h"
#include "Device.h"
#include "Utility/Defines.h"

#endif // RENDER_GRAPH_PCH_H
