
#include "RHI_PCH.h"

#include "VulkanDevice.h"

#include "VulkanHelpers.h"

#include "CommandListPool.h"
#include "CommandQueue.h"
#include "CommandSignature.h"
#include "DescriptorHeap.h"
#include "Fence.h"
#include "PipelineState.h"
#include "QueryHeap.h"
#include "StatisticsQuery.h"
#include "SwapChain.h"
#include "TimestampQuery.h"

namespace rhi::vulkan
{
    namespace
    {
        std::uint64_t GetDeviceLocalMemory(const vk::PhysicalDevice& physicalDevice)
        {
            std::uint64_t localMemory = 0;
            const auto& memoryTypes = physicalDevice.getMemoryProperties().memoryTypes;

            for (const auto& memoryType : memoryTypes)
            {
                if ((memoryType.propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) != vk::MemoryPropertyFlags{})
                {
                    localMemory += physicalDevice.getMemoryProperties().memoryHeaps[memoryType.heapIndex].size;
                }
            }

            return localMemory;
        }
    } // namespace unnamed

    VulkanDevice::VulkanDevice()
        : _instance(CreateInstance())
        , _device(CreateDevice())
    {
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : _instance(std::move(other._instance))
        , _device(std::move(other._device))
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (_device)
        {
            _device.destroy();
        }
        if (_instance)
        {
            _instance.destroy();
        }
        NOT_IMPLEMENTED();
    }

    VulkanDevice& VulkanDevice::operator=(VulkanDevice&& other) noexcept
    {
        if (this != &other)
        {
            _instance = std::move(other._instance);
            _device = std::move(other._device);
        }

        return *this;
    }

    BackendAPI VulkanDevice::GetBackend() const
    {
        return BackendAPI::Vulkan;
    }

    bool VulkanDevice::IsEnhancedBarriersSupported()
    {
        NOT_IMPLEMENTED();
        return false;
    }

    void VulkanDevice::BindSwapChain(SwapChain* swapChain)
    {
        NOT_IMPLEMENTED();
    }

    CommandQueue* VulkanDevice::GetQueue(rhi::CommandListType type)
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    CommandQueue* VulkanDevice::GetGraphicsQueue()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    CommandQueue* VulkanDevice::GetComputeQueue()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    CommandQueue* VulkanDevice::GetCopyQueue()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void VulkanDevice::OnResize(std::uint32_t width, std::uint32_t height)
    {
        NOT_IMPLEMENTED();
    }

    std::shared_ptr<Texture> VulkanDevice::GetBackBuffer()
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Texture>();
    }

    void VulkanDevice::Present()
    {
        NOT_IMPLEMENTED();
    }

    std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDescription& description, ResourceState initialState, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Buffer>();
    }

    std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(void* nativePtr, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Buffer>();
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(const TextureDescription& description, ResourceState initialState, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Texture>();
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(void* nativePtr, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::shared_ptr<Texture>();
    }

    std::unique_ptr<CommandList> VulkanDevice::CreateCommandList(CommandListType type, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<CommandList>();
    }

    std::unique_ptr<CommandListPool> VulkanDevice::CreateCommandListPool()
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<CommandListPool>();
    }

    std::unique_ptr<DescriptorHeap> VulkanDevice::CreateDescriptorHeap(const DescriptorHeapDescription& description, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<DescriptorHeap>();
    }

    std::unique_ptr<QueryHeap> VulkanDevice::CreateQueryHeap(const QueryHeapDescription& description, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<QueryHeap>();
    }

    std::unique_ptr<Fence> VulkanDevice::CreateFence(std::uint64_t initialValue)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<Fence>();
    }

    std::unique_ptr<StatisticsQuery> VulkanDevice::CreateStatisticsQuery(const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<StatisticsQuery>();
    }

    std::unique_ptr<TimestampQuery> VulkanDevice::CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<TimestampQuery>();
    }

    std::unique_ptr<CommandSignature> VulkanDevice::CreateCommandSignature(const std::vector<IndirectArgumentDescription>& arguments, PipelineState* pipelineState, const std::string& name)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<CommandSignature>();
    }

    std::unique_ptr<SwapChain> VulkanDevice::CreateSwapChain(void* windowHandle, std::uint32_t width, std::uint32_t height, bool vSync)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<SwapChain>();
    }

    std::unique_ptr<PipelineState> VulkanDevice::CreatePipelineState(const std::string& filepath)
    {
        NOT_IMPLEMENTED();
        return std::unique_ptr<PipelineState>();
    }

    void VulkanDevice::CreateBufferView(const BufferView& view, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateTextureView(const TextureView& view, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    void VulkanDevice::CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        NOT_IMPLEMENTED();
    }

    std::uint32_t VulkanDevice::GetDescriptorHandleIncrementSize(DescriptorHeapType type) const
    {
        NOT_IMPLEMENTED();
        return std::uint32_t();
    }

    AllocationInfo VulkanDevice::GetAllocationInfo(const BufferDescription& description) const
    {
        NOT_IMPLEMENTED();
        return AllocationInfo();
    }

    AllocationInfo VulkanDevice::GetAllocationInfo(const TextureDescription& description) const
    {
        NOT_IMPLEMENTED();
        return AllocationInfo();
    }

    const AdapterInfo& VulkanDevice::QueryAdapterInfo()
    {
        NOT_IMPLEMENTED();
        return {};
    }

    AllocatorStats VulkanDevice::QueryAllocatorStats() const
    {
        NOT_IMPLEMENTED();
        return AllocatorStats();
    }

    tracking::IGPUCrashTracker* VulkanDevice::GetCrashTracker()
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    void* VulkanDevice::GetNative() const
    {
        NOT_IMPLEMENTED();
        return nullptr;
    }

    vk::Instance VulkanDevice::CreateInstance()
    {
        EnumerateExtensions();

        std::vector<const char*> requiredLayers = 
        {
#if ENABLE_DEVICE_DEBUG
            "VK_LAYER_KHRONOS_validation"
#endif // ENABLE_DEVICE_DEBUG
        };

        if (!CheckLayersSupport(requiredLayers))
        {
            LOG_ERROR("Required Vulkan layers are not supported.");
            return nullptr;
        }

        std::vector<const char*> requiredExtensions =
        {
#if ENABLE_DEVICE_DEBUG
            vk::EXTDebugUtilsExtensionName
#endif // ENABLE_DEVICE_DEBUG
        };

        if (!CheckExtensionsSupport(requiredExtensions))
        {
            LOG_ERROR("Required Vulkan extensions are not supported.");
            return nullptr;
        }

        constexpr vk::ApplicationInfo appInfo =
        { 
            .pApplicationName = "Equinox Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Equinox Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = vk::ApiVersion14
        };

        const vk::InstanceCreateInfo createInfo =
        {
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data()
        };

        auto [result, instance] = vk::createInstance(createInfo);
        VK_CHECK(result, "Failed to create Vulkan instance");

        return instance;
    }

    vk::Device VulkanDevice::CreateDevice()
    {
        auto [enumerateResult, physicalDevices] = _instance.enumeratePhysicalDevices();
        VK_CHECK(enumerateResult, "Failed to enumerate Vulkan physical devices");

        std::uint64_t maxMemory = 0;
        vk::PhysicalDevice* selectedDevice = nullptr;

        for (auto& physicalDevice : physicalDevices)
        {
            vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

            if (properties.deviceType == vk::PhysicalDeviceType::eCpu)
            {
                continue; // Skip software adapters
            }

            const std::uint64_t memory = GetDeviceLocalMemory(physicalDevice);
            if (memory > maxMemory)
            {
                maxMemory = memory;
                selectedDevice = &physicalDevice;
            }
        }
        ASSERT(selectedDevice != nullptr, "No suitable Vulkan physical device found.");

        vk::DeviceCreateInfo deviceCreateInfo = {};
        auto [createResult, device] = selectedDevice->createDevice(deviceCreateInfo);
        VK_CHECK(createResult, "Failed to create Vulkan logical device");

        return device;
    }

    void VulkanDevice::EnumerateExtensions() const
    {
        auto [result, availableExtensions] = vk::enumerateInstanceExtensionProperties();
        VK_CHECK(result, "Failed to enumerate Vulkan instance extensions");

        LOG_DEBUG("Available Vulkan Extensions:");
        for (const auto& extension : availableExtensions)
        {
            LOG_DEBUG(" - {}", extension.extensionName.data());
        }
    }

    bool VulkanDevice::CheckExtensionsSupport(const std::vector<const char*>& extensions) const
    {
        auto [result, availableExtensions] = vk::enumerateInstanceExtensionProperties();
        VK_CHECK(result, "Failed to enumerate Vulkan instance extensions");

        auto unsupportedExtensionIt = std::find_if(extensions.begin(), extensions.end(),
            [&](const char* extension) {
                return std::none_of(availableExtensions.begin(), availableExtensions.end(),
                    [&](const vk::ExtensionProperties& availableExtension) {
                        return strcmp(extension, availableExtension.extensionName) == 0;
                    });
            });

        if (unsupportedExtensionIt != extensions.end())
        {
            LOG_ERROR("Unsupported Vulkan extension: {}", *unsupportedExtensionIt);
            return false;
        }

        return true;
    }

    bool VulkanDevice::CheckLayersSupport(const std::vector<const char*>& layers) const
    {
        auto [result, availableLayers] = vk::enumerateInstanceLayerProperties();
        VK_CHECK(result, "Failed to enumerate Vulkan instance layers");

        auto unsupportedLayerIt = std::find_if(layers.begin(), layers.end(),
            [&](const char* layer) {
                return std::none_of(availableLayers.begin(), availableLayers.end(),
                    [&](const vk::LayerProperties& availableLayer) {
                        return strcmp(layer, availableLayer.layerName) == 0;
                    });
            });

        if (unsupportedLayerIt != layers.end())
        {
            LOG_ERROR("Unsupported Vulkan layer: {}", *unsupportedLayerIt);
            return false;
        }

        return true;
    }
} // namespace rhi::vulkan
