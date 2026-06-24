
#include "RHI_PCH.h"

#include "VulkanDevice.h"

#include "VulkanBuffer.h"
#include "VulkanCommandList.h"
#include "VulkanCommandListPool.h"
#include "VulkanCommandQueue.h"
#include "VulkanHelpers.h"
#include "VulkanPipelineState.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"

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

#include "IGPUCrashTracker.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace rhi::vulkan
{
    namespace
    {
        // Instance-scoped extensions
        const std::vector<const char*> kInstanceExtensions =
        {
#if ENABLE_DEVICE_DEBUG
            vk::EXTDebugUtilsExtensionName,
#endif // ENABLE_DEVICE_DEBUG
        };

        // Device-scoped extensions
        const std::vector<const char*> kDeviceExtensions =
        {
            vk::KHRSwapchainExtensionName,
        };

        // Instance-scoped validation layers
        const std::vector<const char*> kValidationLayers =
        {
#if ENABLE_DEVICE_DEBUG
            "VK_LAYER_KHRONOS_validation",
#endif // ENABLE_DEVICE_DEBUG
        };

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*)
        {
            switch (severity)
            {
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
                LOG_DEBUG("Vulkan validation layer: type {} msg: {}", to_string(type), pCallbackData->pMessage);
                break;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
                LOG_INFO("Vulkan validation layer: type {} msg: {}", to_string(type), pCallbackData->pMessage);
                break;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
                LOG_WARNING("Vulkan validation layer: type {} msg: {}", to_string(type), pCallbackData->pMessage);
                break;
            case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
                LOG_ERROR("Vulkan validation layer: type {} msg: {}", to_string(type), pCallbackData->pMessage);
                break;
            default:
                break;
            }

            return vk::False;
        }

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
        : _instance(nullptr)
        , _physicalDevice(nullptr)
        , _logicalDevice(nullptr)
        , _graphicsQueue(nullptr)
        , _computeQueue(nullptr)
#if ENABLE_DEVICE_DEBUG
        , _debugMessenger(nullptr)
#endif // ENABLE_DEVICE_DEBUG
    {
        _instance = CreateInstance();
#if ENABLE_DEVICE_DEBUG
        _debugMessenger = SetupDebugMessenger();
#endif // ENABLE_DEVICE_DEBUG
        _logicalDevice = CreateDevice();

        LOG_INFO("Vulkan instance created successfully.");
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : _instance(std::exchange(other._instance, nullptr))
        , _physicalDevice(std::exchange(other._physicalDevice, nullptr))
        , _logicalDevice(std::exchange(other._logicalDevice, nullptr))
        , _graphicsQueue(std::exchange(other._graphicsQueue, nullptr))
#if ENABLE_DEVICE_DEBUG
        , _debugMessenger(std::exchange(other._debugMessenger, nullptr))
#endif // ENABLE_DEVICE_DEBUG
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (_logicalDevice)
        {
            _logicalDevice.destroy();
        }
#if ENABLE_DEVICE_DEBUG
        if (_debugMessenger)
        {
            _instance.destroyDebugUtilsMessengerEXT(_debugMessenger);
        }
#endif // ENABLE_DEVICE_DEBUG
        if (_instance)
        {
            _instance.destroy();
        }
    }

    VulkanDevice& VulkanDevice::operator=(VulkanDevice&& other) noexcept
    {
        if (this != &other)
        {
            _instance = std::exchange(other._instance, nullptr);
            _physicalDevice = std::exchange(other._physicalDevice, nullptr);
            _logicalDevice = std::exchange(other._logicalDevice, nullptr);
            _graphicsQueue = std::exchange(other._graphicsQueue, nullptr);
#if ENABLE_DEVICE_DEBUG
            _debugMessenger = std::exchange(other._debugMessenger, nullptr);
#endif // ENABLE_DEVICE_DEBUG
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

    CommandQueue* VulkanDevice::GetGraphicsQueue()
    {
        return _graphicsQueue.get();
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
        return std::unique_ptr<Buffer>(new VulkanBuffer(this, description, initialState, name));
    }

    std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<Buffer>(new VulkanBuffer(this, nativePtr, name));
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(const TextureDescription& description, ResourceState initialState, const std::string& name)
    {
        return std::unique_ptr<Texture>(new VulkanTexture(this, description, initialState, name));
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<Texture>(new VulkanTexture(this, nativePtr, name));
    }

    std::unique_ptr<CommandList> VulkanDevice::CreateCommandList(CommandListType type, const std::string& name)
    {
        return std::unique_ptr<CommandList>(new VulkanCommandList(this, type, name));
    }

    std::unique_ptr<CommandListPool> VulkanDevice::CreateCommandListPool()
    {
        return std::unique_ptr<CommandListPool>(new VulkanCommandListPool(this));
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
        return std::unique_ptr<SwapChain>(new VulkanSwapChain(this, (HWND)windowHandle, width, height, vSync));
    }

    std::unique_ptr<PipelineState> VulkanDevice::CreatePipelineState(const std::string& filepath)
    {
        return std::unique_ptr<PipelineState>(new VulkanPipelineState(this, filepath));
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
        return _crashTracker.get();
    }

    void* VulkanDevice::GetNative() const
    {
        return VulkanNative(_logicalDevice);
    }

    vk::Instance VulkanDevice::GetVulkanInstance() const
    {
        return _instance;
    }

    vk::PhysicalDevice VulkanDevice::GetPhysicalDevice() const
    {
        return _physicalDevice;
    }

    vk::Instance VulkanDevice::CreateInstance()
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        EnumerateExtensions();

        if (!CheckLayersSupport(kValidationLayers))
        {
            LOG_ERROR("Required Vulkan layers are not supported.");
            return nullptr;
        }

        if (!CheckExtensionsSupport(kInstanceExtensions))
        {
            LOG_ERROR("Required Vulkan instance extensions are not supported.");
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
            .enabledLayerCount = static_cast<std::uint32_t>(kValidationLayers.size()),
            .ppEnabledLayerNames = kValidationLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(kInstanceExtensions.size()),
            .ppEnabledExtensionNames = kInstanceExtensions.data()
        };

        auto [result, instance] = vk::createInstance(createInfo);
        VK_CHECK(result, "Failed to create Vulkan instance");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

        return instance;
    }

    vk::Device VulkanDevice::CreateDevice()
    {
        auto [enumerateResult, physicalDevices] = _instance.enumeratePhysicalDevices();
        VK_CHECK(enumerateResult, "Failed to enumerate Vulkan physical devices");

        std::uint64_t maxMemory = 0;

        for (auto& physicalDevice : physicalDevices)
        {
            vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

            if (properties.deviceType == vk::PhysicalDeviceType::eCpu)
            {
                continue; // Skip software adapters
            }

            if (!CheckFeatureSupport(physicalDevice))
            {
                continue; // Skip devices that do not support required features
            }

            const std::uint64_t memory = GetDeviceLocalMemory(physicalDevice);
            if (memory > maxMemory)
            {
                maxMemory = memory;
                _physicalDevice = physicalDevice;
            }
        }
        ASSERT(_physicalDevice, "No suitable Vulkan physical device found.");

        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();
        auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, 
            [](auto const& qfp) 
            { return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0); }
        );
        const std::uint32_t graphicsQueueFamilyIndex = static_cast<std::uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo = { .queueFamilyIndex = graphicsQueueFamilyIndex };

        // Create a chain of feature structures
        vk::StructureChain<vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain = {
                {},                                    // vk::PhysicalDeviceFeatures2 (empty for now)
                {.shaderDrawParameters = true},        // Enable shader draw parameters from Vulkan 1.1
                {.dynamicRendering = true},            // Enable dynamic rendering from Vulkan 1.3
                {.extendedDynamicState = true}         // Enable extended dynamic state from the extension
        };

        vk::DeviceCreateInfo deviceCreateInfo = 
        {
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &deviceQueueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size()),
            .ppEnabledExtensionNames = kDeviceExtensions.data()
        };

        auto [createResult, device] = _physicalDevice.createDevice(deviceCreateInfo);
        VK_CHECK(createResult, "Failed to create Vulkan logical device");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

        // Assign the member before creating the queue: VulkanCommandQueue reads the
        // logical device back through _device->GetNative().
        _logicalDevice = device;
        _graphicsQueue = std::unique_ptr<VulkanCommandQueue>(new VulkanCommandQueue(this, rhi::CommandListType::Graphics, graphicsQueueFamilyIndex));

        return device;
    }

#if ENABLE_DEVICE_DEBUG
    vk::DebugUtilsMessengerEXT VulkanDevice::SetupDebugMessenger()
    {
        using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

        const vk::DebugUtilsMessengerCreateInfoEXT createInfo =
        {
            .messageSeverity = Severity::eVerbose | Severity::eInfo | Severity::eWarning | Severity::eError,
            .messageType = Type::eGeneral | Type::eValidation | Type::ePerformance,
            .pfnUserCallback = debugCallback,
        };

        auto [result, debugMessenger] = _instance.createDebugUtilsMessengerEXT(createInfo);
        VK_CHECK(result, "Failed to create Vulkan debug messenger");

        return debugMessenger;
    }
#endif // ENABLE_DEVICE_DEBUG

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

    bool VulkanDevice::CheckFeatureSupport(const vk::PhysicalDevice& physicalDevice) const
    {
        auto features = physicalDevice.template getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

        bool supportsRequiredFeatures = features.template get<
            vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        return supportsRequiredFeatures;
    }
} // namespace rhi::vulkan
