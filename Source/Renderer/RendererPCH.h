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

#include <DirectXMath.h>

#include <filesystem>
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cstdint>
#include <string>
#include <queue>
#include <chrono>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>
#include <map>
#include <unordered_map>

#include "Logger/Logger.h"

#include "RHI/Buffer.h"
#include "RHI/Texture.h"
#include "RHI/CommandList.h"
#include "RHI/Device.h"
#include "RHI/GPUEvent.h"

#include "Helpers/Defines.h"
#include "Utility/Helpers.h"

#include "RenderGraph/RenderGraphResourceId.h"
