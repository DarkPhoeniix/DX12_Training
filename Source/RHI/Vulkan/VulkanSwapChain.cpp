
#include "RHI_PCH.h"

#include "VulkanSwapChain.h"

#include "VulkanCommandQueue.h"
#include "VulkanHelpers.h"

#include "CommandQueue.h"
#include "CommandList.h"
#include "DescriptorHeap.h"
#include "Texture.h"

namespace rhi::vulkan
{
    VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
        : _device(device)
        , _surface(CreateSurface(windowHandle))
        , _swapChain(CreateSwapChain(windowHandle, width, height, vSync))
    {
    }

    VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept
        : _device(std::move(other._device))
        , _surface(other._surface)
        , _swapChain(other._swapChain)
    {
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        if (_swapChain)
        {
            vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
            logicalDevice.destroySwapchainKHR(_swapChain);
        }

        if (_surface)
        {
            _device->GetVulkanInstance().destroySurfaceKHR(_surface);
        }
    }

    VulkanSwapChain& VulkanSwapChain::operator=(VulkanSwapChain&& other) noexcept
    {
        if (this != &other)
        {
            _device = std::move(other._device);
            _surface = other._surface;
            _swapChain = other._swapChain;
        }

        return *this;
    }

    std::shared_ptr<rhi::Texture> VulkanSwapChain::GetBuffer(std::uint32_t index)
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<rhi::Texture>();
    }

    std::shared_ptr<rhi::Texture> VulkanSwapChain::GetBackBuffer()
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Texture>();
    }

    std::uint32_t VulkanSwapChain::Present()
    {
        NOT_IMPLEMENTED();
        return std::uint32_t();
    }

    void VulkanSwapChain::OnResize(std::uint32_t width, std::uint32_t height)
    {
        NOT_IMPLEMENTED();
    }

    rhi::ScissorRect VulkanSwapChain::GetDesktopCoordinates()
    {
        NOT_IMPLEMENTED();
        return rhi::ScissorRect();
    }

    void* VulkanSwapChain::GetNative() const
    {
        return VulkanNative(_swapChain);
    }

    vk::SurfaceKHR VulkanSwapChain::CreateSurface(HWND windowHandle)
    {
        vk::Win32SurfaceCreateInfoKHR createInfo =
        {
            .hinstance = GetModuleHandle(nullptr),
            .hwnd = windowHandle
        };

        auto [result, surface] = _device->GetVulkanInstance().createWin32SurfaceKHR(createInfo);
        VK_CHECK(result, "Failed to create Win32 surface");

        return surface;
    }

    vk::SwapchainKHR VulkanSwapChain::CreateSwapChain(HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        vk::PhysicalDevice physicalDevice = _device->GetPhysicalDevice();
        const std::uint32_t graphicsFamily = static_cast<VulkanCommandQueue*>(_device->GetGraphicsQueue())->GetQueueFamilyIndex();

        // The graphics queue family must be able to present to this surface
        auto [supportResult, presentSupported] = physicalDevice.getSurfaceSupportKHR(graphicsFamily, _surface);
        VK_CHECK(supportResult, "Failed to query surface present support");
        ASSERT(presentSupported, "Graphics queue family does not support presentation to this surface.");

        auto [capsResult, capabilities] = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
        VK_CHECK(capsResult, "Failed to query surface capabilities");

        auto [formatsResult, formats] = physicalDevice.getSurfaceFormatsKHR(_surface);
        VK_CHECK(formatsResult, "Failed to query surface formats");

        auto [presentModesResult, presentModes] = physicalDevice.getSurfacePresentModesKHR(_surface);
        VK_CHECK(presentModesResult, "Failed to query surface present modes");

        // Prefer 8-bit RGBA sRGB; fall back to whatever the surface offers first
        vk::SurfaceFormatKHR surfaceFormat = formats.front();
        for (const vk::SurfaceFormatKHR& format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                surfaceFormat = format;
                break;
            }
        }

        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
        if (!vSync)
        {
            for (vk::PresentModeKHR mode : presentModes)
            {
                if (mode == vk::PresentModeKHR::eMailbox)
                {
                    presentMode = mode;
                    break;
                }
            }
        }

        vk::Extent2D extent = capabilities.currentExtent;
        if (capabilities.currentExtent.width == UINT32_MAX)
        {
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        // Aim for triple buffering, clamped to what the surface allows (maxImageCount == 0 means no limit).
        const std::uint32_t maxImageCount = capabilities.maxImageCount > 0 ? capabilities.maxImageCount : UINT32_MAX;
        const std::uint32_t imageCount = std::clamp(rhi::BACK_BUFFER_COUNT, capabilities.minImageCount, maxImageCount);

        const vk::SwapchainCreateInfoKHR createInfo =
        {
            .surface = _surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .preTransform = capabilities.currentTransform,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = presentMode,
            .clipped = vk::True,
            .oldSwapchain = nullptr
        };

        auto [result, swapChain] = logicalDevice.createSwapchainKHR(createInfo);
        VK_CHECK(result, "Failed to create Vulkan swapchain");

        return swapChain;
    }
} // namespace rhi::vulkan
