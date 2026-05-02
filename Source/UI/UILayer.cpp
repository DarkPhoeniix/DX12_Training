
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "UILayer.h"

#include "Logger/Logger.h"

#include "RHI/Device.h"

#if USE_D3D12
#include "D3D12/D3D12UILayer.h"
#endif // USE_D3D12

#if USE_VULKAN
#include "Vulkan/VkUILayer.h"
#endif // USE_VULKAN

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

namespace ui
{
    std::unique_ptr<UILayer> CreateUILayer(rhi::Device* device, void* windowHandle)
    {
        std::unique_ptr<UILayer> uiLayer = nullptr;

        switch (device->GetBackend())
        {
#if USE_D3D12
        case rhi::BackendAPI::D3D12:
            uiLayer = std::make_unique<D3D12UILayer>(device, windowHandle);
            break;
#endif // USE_D3D12

#if USE_VULKAN
        case rhi::BackendAPI::Vulkan:
            uiLayer = std::make_unique<ui::VkUILayer>(device, windowHandle);
            break;
#endif // USE_VULKAN

        default:
            UNREACHABLE("Unsupported backend API.");
            break;
        }

        return uiLayer;
    }
} // namespace ui
