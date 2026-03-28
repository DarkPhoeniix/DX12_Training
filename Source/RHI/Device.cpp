
#include "RHI_PCH.h"

#include "Device.h"

#if USE_D3D12
#include "D3D12/D3D12Device.h"
#endif

#if USE_VULKAN
#include "Vulkan/VkDevice.h"
#endif

namespace rhi
{
    static std::unique_ptr<Device> CreateDevice(BackendAPI backend)
    {
        std::unique_ptr<Device> device = nullptr;

        switch (backend)
        {
#if USE_D3D12
        case BackendAPI::D3D12:
            device = std::make_unique<d3d12::D3D12Device>();
            break;
#endif
#if USE_VULKAN
        case BackendAPI::Vulkan:
            device = std::make_unique<vk::VkDevice>();
            break;
#endif
        default:
            LOG_CRITICAL("Unsupported backend API!");
            break;
        }

        return device;
    }
} // namespace rhi
