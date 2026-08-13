
#include "RHI_PCH.h"

#include "VulkanSwapChain.h"

#include "VulkanCommandQueue.h"
#include "VulkanHelpers.h"
#include "VulkanTexture.h"

#include "CommandQueue.h"
#include "CommandList.h"
#include "DescriptorHeap.h"
#include "Texture.h"

namespace rhi::vulkan
{
    VulkanSwapChain::VulkanSwapChain(VulkanDevice* device, HWND windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
        : _device(device)
        , _windowHandle(windowHandle)
        , _width(width)
        , _height(height)
        , _vSync(vSync)
        , _tearingSupport(false)
        , _presentQueue(VulkanCast<vk::Queue>(device->GetGraphicsQueue()->GetNative()))
        , _imageFormat(vk::Format::eUndefined)
        , _surface(CreateSurface(windowHandle))
        , _swapChain(CreateSwapChain(nullptr))
        , _currentBackBufferIndex(0)
        , _semaphoreIndex(0)
        , _acquireSemaphoreConsumed(false)
    {
        CreateBackBuffers();
        CreateSemaphores();

        AcquireNextImage();
    }

    VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept
        : _device(std::exchange(other._device, nullptr))
        , _windowHandle(std::exchange(other._windowHandle, nullptr))
        , _width(other._width)
        , _height(other._height)
        , _vSync(other._vSync)
        , _tearingSupport(other._tearingSupport)
        , _presentQueue(std::exchange(other._presentQueue, nullptr))
        , _surface(std::exchange(other._surface, nullptr))
        , _swapChain(std::exchange(other._swapChain, nullptr))
        , _imageFormat(other._imageFormat)
        , _backBuffers(std::move(other._backBuffers))
        , _currentBackBufferIndex(other._currentBackBufferIndex)
        , _acquireSemaphores(std::move(other._acquireSemaphores))
        , _renderFinishedSemaphores(std::move(other._renderFinishedSemaphores))
        , _semaphoreIndex(other._semaphoreIndex)
        , _acquireSemaphoreConsumed(other._acquireSemaphoreConsumed)
    {
    }

    VulkanSwapChain::~VulkanSwapChain()
    {
        if (!_device)
        {
            return;
        }

        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        const vk::Result waitResult = logicalDevice.waitIdle();
        VK_CHECK(waitResult, "Failed to wait for device idle before swapchain destruction");

        DestroySemaphores();
        _backBuffers.clear();

        if (_swapChain)
        {
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
            _device = std::exchange(other._device, nullptr);
            _windowHandle = std::exchange(other._windowHandle, nullptr);
            _width = other._width;
            _height = other._height;
            _vSync = other._vSync;
            _tearingSupport = other._tearingSupport;
            _presentQueue = std::exchange(other._presentQueue, nullptr);
            _surface = std::exchange(other._surface, nullptr);
            _swapChain = std::exchange(other._swapChain, nullptr);
            _imageFormat = other._imageFormat;
            _backBuffers = std::move(other._backBuffers);
            _currentBackBufferIndex = other._currentBackBufferIndex;
            _acquireSemaphores = std::move(other._acquireSemaphores);
            _renderFinishedSemaphores = std::move(other._renderFinishedSemaphores);
            _semaphoreIndex = other._semaphoreIndex;
            _acquireSemaphoreConsumed = other._acquireSemaphoreConsumed;
        }

        return *this;
    }

    std::shared_ptr<Texture> VulkanSwapChain::GetBuffer(std::uint32_t index)
    {
        ASSERT(index < _backBuffers.size(), "Swapchain buffer index is out of range.");
        return _backBuffers[index];
    }

    std::shared_ptr<Texture> VulkanSwapChain::GetBackBuffer()
    {
        return _backBuffers[_currentBackBufferIndex];
    }

    std::uint32_t VulkanSwapChain::Present()
    {
        const vk::PresentInfoKHR presentInfo =
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &_renderFinishedSemaphores[_currentBackBufferIndex],
            .swapchainCount = 1,
            .pSwapchains = &_swapChain,
            .pImageIndices = &_currentBackBufferIndex
        };

        const vk::Result presentResult = _presentQueue.presentKHR(presentInfo);
        if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
        {
            RecreateSwapChain();
        }
        else
        {
            VK_CHECK(presentResult, "Failed to present swapchain image");
        }

        return AcquireNextImage();
    }

    void VulkanSwapChain::OnResize(std::uint32_t width, std::uint32_t height)
    {
        if (_width != width || _height != height)
        {
            _width = std::max((std::uint32_t)1, width);
            _height = std::max((std::uint32_t)1, height);

            RecreateSwapChain();
            AcquireNextImage();
        }
    }

    ScissorRect VulkanSwapChain::GetDesktopCoordinates()
    {
        RECT rect;
        GetWindowRect(_windowHandle, &rect);
        
        ScissorRect scissorRect =
        {
            .Left = rect.left,
            .Top = rect.top,
            .Right = rect.right,
            .Bottom = rect.bottom
        };

        return scissorRect;
    }

    void* VulkanSwapChain::GetNative() const
    {
        return VulkanNative(_swapChain);
    }

    bool VulkanSwapChain::OwnsTexture(const Texture* texture) const
    {
        return std::ranges::any_of(_backBuffers,
            [texture](const std::shared_ptr<Texture>& backBuffer) { return backBuffer.get() == texture; });
    }

    vk::Semaphore VulkanSwapChain::ConsumeAcquireSemaphore()
    {
        if (_acquireSemaphoreConsumed)
        {
            return nullptr;
        }

        _acquireSemaphoreConsumed = true;
        return _acquireSemaphores[_semaphoreIndex];
    }

    vk::Semaphore VulkanSwapChain::GetRenderFinishedSemaphore() const
    {
        return _renderFinishedSemaphores[_currentBackBufferIndex];
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

    vk::SwapchainKHR VulkanSwapChain::CreateSwapChain(vk::SwapchainKHR oldSwapChain)
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        vk::PhysicalDevice physicalDevice = _device->GetPhysicalDevice();
        const std::uint32_t graphicsFamily = static_cast<VulkanCommandQueue*>(_device->GetGraphicsQueue())->GetQueueFamilyIndex();

        auto [supportResult, presentSupported] = physicalDevice.getSurfaceSupportKHR(graphicsFamily, _surface);
        VK_CHECK(supportResult, "Failed to query surface present support");
        ASSERT(presentSupported, "Graphics queue family does not support presentation to this surface.");

        auto [capsResult, capabilities] = physicalDevice.getSurfaceCapabilitiesKHR(_surface);
        VK_CHECK(capsResult, "Failed to query surface capabilities");

        auto [formatsResult, formats] = physicalDevice.getSurfaceFormatsKHR(_surface);
        VK_CHECK(formatsResult, "Failed to query surface formats");

        auto [presentModesResult, presentModes] = physicalDevice.getSurfacePresentModesKHR(_surface);
        VK_CHECK(presentModesResult, "Failed to query surface present modes");

        vk::SurfaceFormatKHR surfaceFormat = formats.front();
        for (const vk::SurfaceFormatKHR& format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                surfaceFormat = format;
                break;
            }
        }

        _tearingSupport = std::ranges::find(presentModes, vk::PresentModeKHR::eImmediate) != presentModes.end();

        vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
        if (!_vSync)
        {
            if (std::ranges::find(presentModes, vk::PresentModeKHR::eMailbox) != presentModes.end())
            {
                presentMode = vk::PresentModeKHR::eMailbox;
            }
            else if (_tearingSupport)
            {
                presentMode = vk::PresentModeKHR::eImmediate;
            }
        }

        vk::Extent2D extent = capabilities.currentExtent;
        if (capabilities.currentExtent.width == UINT32_MAX)
        {
            extent.width = std::clamp(_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        }

        // The surface can refuse the requested size, so keep the members in step with it
        _width = extent.width;
        _height = extent.height;
        _imageFormat = surfaceFormat.format;

        // Aim for triple buffering, clamped to what the surface allows
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
            .oldSwapchain = oldSwapChain
        };

        auto [result, swapChain] = logicalDevice.createSwapchainKHR(createInfo);
        VK_CHECK(result, "Failed to create Vulkan swapchain");

        return swapChain;
    }

    void VulkanSwapChain::CreateBackBuffers()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        auto [result, images] = logicalDevice.getSwapchainImagesKHR(_swapChain);
        VK_CHECK(result, "Failed to retrieve swapchain images");

        const TextureDescription description =
        {
            .Width = _width,
            .Height = _height,
            .Format = GetRHIFormat(_imageFormat),
            .Dimension = TextureDimension::Texture2D,
            .Flags = ResourceFlags::AllowRenderTarget
        };

        _backBuffers.reserve(images.size());
        for (std::size_t i = 0; i < images.size(); ++i)
        {
            std::shared_ptr<Texture> backBuffer = _device->CreateTexture(VulkanNative(images[i]), "BackBuffer" + std::to_string(i));
            static_cast<VulkanTexture*>(backBuffer.get())->_description = description;

            _backBuffers.push_back(std::move(backBuffer));
        }
    }

    void VulkanSwapChain::CreateSemaphores()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());
        constexpr vk::SemaphoreCreateInfo createInfo = {};

        _acquireSemaphores.resize(BACK_BUFFER_COUNT);
        _renderFinishedSemaphores.resize(_backBuffers.size());

        for (vk::Semaphore& semaphore : _acquireSemaphores)
        {
            auto [result, created] = logicalDevice.createSemaphore(createInfo);
            VK_CHECK(result, "Failed to create swapchain acquire semaphore");
            semaphore = created;
        }

        for (vk::Semaphore& semaphore : _renderFinishedSemaphores)
        {
            auto [result, created] = logicalDevice.createSemaphore(createInfo);
            VK_CHECK(result, "Failed to create swapchain render finished semaphore");
            semaphore = created;
        }

        _semaphoreIndex = 0;
    }

    void VulkanSwapChain::DestroySemaphores()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        for (vk::Semaphore semaphore : _acquireSemaphores)
        {
            logicalDevice.destroySemaphore(semaphore);
        }

        for (vk::Semaphore semaphore : _renderFinishedSemaphores)
        {
            logicalDevice.destroySemaphore(semaphore);
        }

        _acquireSemaphores.clear();
        _renderFinishedSemaphores.clear();
    }

    void VulkanSwapChain::RecreateSwapChain()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        const vk::Result waitResult = logicalDevice.waitIdle();
        VK_CHECK(waitResult, "Failed to wait for device idle before swapchain recreation");

        DestroySemaphores();
        _backBuffers.clear();

        const vk::SwapchainKHR oldSwapChain = _swapChain;
        _swapChain = CreateSwapChain(oldSwapChain);
        logicalDevice.destroySwapchainKHR(oldSwapChain);

        CreateBackBuffers();
        CreateSemaphores();

        _currentBackBufferIndex = 0;
    }

    std::uint32_t VulkanSwapChain::AcquireNextImage()
    {
        vk::Device logicalDevice = VulkanCast<vk::Device>(_device->GetNative());

        _semaphoreIndex = (_semaphoreIndex + 1) % static_cast<std::uint32_t>(_acquireSemaphores.size());

        auto acquired = logicalDevice.acquireNextImageKHR(_swapChain, UINT64_MAX, _acquireSemaphores[_semaphoreIndex]);
        if (acquired.result == vk::Result::eErrorOutOfDateKHR)
        {
            RecreateSwapChain();
            acquired = logicalDevice.acquireNextImageKHR(_swapChain, UINT64_MAX, _acquireSemaphores[_semaphoreIndex]);
        }

        // eSuboptimalKHR still yields a usable image; presenting it is what triggers the rebuild
        if (acquired.result != vk::Result::eSuboptimalKHR)
        {
            VK_CHECK(acquired.result, "Failed to acquire next swapchain image");
        }

        _currentBackBufferIndex = acquired.value;
        _acquireSemaphoreConsumed = false;

        return _currentBackBufferIndex;
    }
} // namespace rhi::vulkan
