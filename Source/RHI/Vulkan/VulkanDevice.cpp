
#include "RHI_PCH.h"

#include "VulkanDevice.h"

#include "VulkanBuffer.h"
#include "VulkanCommandList.h"
#include "VulkanCommandListPool.h"
#include "VulkanCommandQueue.h"
#include "VulkanDescriptorHeap.h"
#include "VulkanHelpers.h"
#include "VulkanPipelineState.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanTimestampQuery.h"
#include "VulkanStatisticsQuery.h"
#include "VulkanFence.h"
#include "VulkanQueryHeap.h"

#include "BufferView.h"
#include "CommandListPool.h"
#include "CommandQueue.h"
#include "CommandSignature.h"
#include "DescriptorHeap.h"
#include "Fence.h"
#include "PipelineState.h"
#include "QueryHeap.h"
#include "StatisticsQuery.h"
#include "SwapChain.h"
#include "TextureView.h"
#include "TimestampQuery.h"

#include "IGPUCrashTracker.h"

#include <vma/vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace rhi::vulkan
{
    namespace
    {
        // Instance-scoped extensions
        const std::vector<const char*> kInstanceExtensions =
        {
            vk::KHRSurfaceExtensionName,
            vk::KHRWin32SurfaceExtensionName,
#if ENABLE_DEVICE_DEBUG
            vk::EXTDebugUtilsExtensionName,
#endif // ENABLE_DEVICE_DEBUG
        };

        // Device-scoped extensions
        const std::vector<const char*> kDeviceExtensions =
        {
            vk::KHRSwapchainExtensionName,
            vk::KHRDynamicRenderingExtensionName,   // core in Vulkan 1.3
            vk::EXTDescriptorIndexingExtensionName, // core in Vulkan 1.2
            vk::EXTMutableDescriptorTypeExtensionName,
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

        constexpr std::uint32_t kInvalidQueueFamily = std::uint32_t(-1);

        std::uint32_t FindQueueFamilyIndex(const std::vector<vk::QueueFamilyProperties>& families, vk::QueueFlags required, vk::QueueFlags avoided)
        {
            std::uint32_t result = kInvalidQueueFamily;

            for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(families.size()); ++i)
            {
                const vk::QueueFlags flags = families[i].queueFlags;
                if ((flags & required) != required)
                {
                    continue;
                }

                if ((flags & avoided) == vk::QueueFlags{})
                {
                    result = i;
                    break;
                }

                if (result == kInvalidQueueFamily)
                {
                    result = i;
                }
            }

            return result;
        }
    } // namespace unnamed

    VulkanDevice::VulkanDevice()
        : _instance(nullptr)
        , _physicalDevice(nullptr)
        , _logicalDevice(nullptr)
        , _allocator(nullptr)
        , _swapChain(nullptr)
        , _graphicsQueue(nullptr)
        , _computeQueue(nullptr)
        , _copyQueue(nullptr)
#if ENABLE_DEVICE_DEBUG
        , _debugMessenger(nullptr)
#endif // ENABLE_DEVICE_DEBUG
        , _crashTracker(tracking::IGPUCrashTracker::Create())
    {
        _instance = CreateInstance();
#if ENABLE_DEVICE_DEBUG
        _debugMessenger = SetupDebugMessenger();
#endif // ENABLE_DEVICE_DEBUG
        _logicalDevice = CreateDevice();
        if (!_logicalDevice)
        {
            return;
        }

        _allocator = CreateAllocator();

        LOG_INFO("Vulkan instance created successfully.");
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : _instance(std::exchange(other._instance, nullptr))
        , _physicalDevice(std::exchange(other._physicalDevice, nullptr))
        , _logicalDevice(std::exchange(other._logicalDevice, nullptr))
        , _graphicsQueue(std::exchange(other._graphicsQueue, nullptr))
        , _computeQueue(std::exchange(other._computeQueue, nullptr))
        , _copyQueue(std::exchange(other._copyQueue, nullptr))
#if ENABLE_DEVICE_DEBUG
        , _debugMessenger(std::exchange(other._debugMessenger, nullptr))
#endif // ENABLE_DEVICE_DEBUG
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (_logicalDevice)
        {
            const vk::Result waitResult = _logicalDevice.waitIdle();
            VK_CHECK(waitResult, "Failed to wait for device idle before destruction");

            if (_allocator)
            {
                vmaDestroyAllocator(_allocator);
            }

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
            _computeQueue = std::exchange(other._computeQueue, nullptr);
            _copyQueue = std::exchange(other._copyQueue, nullptr);
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
        _swapChain = swapChain;
    }

    CommandQueue* VulkanDevice::GetGraphicsQueue()
    {
        return _graphicsQueue.get();
    }

    CommandQueue* VulkanDevice::GetComputeQueue()
    {
        return _computeQueue.get();
    }

    CommandQueue* VulkanDevice::GetCopyQueue()
    {
        return _copyQueue.get();
    }

    void VulkanDevice::OnResize(std::uint32_t width, std::uint32_t height)
    {
        _swapChain->OnResize(width, height);
    }

    std::shared_ptr<Texture> VulkanDevice::GetBackBuffer()
    {
        return _swapChain->GetBackBuffer();
    }

    void VulkanDevice::Present()
    {
        _swapChain->Present();
    }

    std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(const BufferDescription& description, ResourceState initialState, const std::string& name)
    {
        return std::unique_ptr<Buffer>(new VulkanBuffer(this, _allocator, description, initialState, name));
    }

    std::shared_ptr<Buffer> VulkanDevice::CreateBuffer(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<Buffer>(new VulkanBuffer(this, VulkanCast<vk::Buffer>(nativePtr), name));
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(const TextureDescription& description, ResourceState initialState, const std::string& name)
    {
        return std::unique_ptr<Texture>(new VulkanTexture(this, _allocator, description, initialState, name));
    }

    std::shared_ptr<Texture> VulkanDevice::CreateTexture(void* nativePtr, const std::string& name)
    {
        return std::unique_ptr<Texture>(new VulkanTexture(this, VulkanCast<vk::Image>(nativePtr), name));
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
        return std::unique_ptr<DescriptorHeap>(new VulkanDescriptorHeap(this, description, name));
    }

    std::unique_ptr<QueryHeap> VulkanDevice::CreateQueryHeap(const QueryHeapDescription& description, const std::string& name)
    {
        return std::unique_ptr<QueryHeap>(new VulkanQueryHeap(this, description, name));
    }

    std::unique_ptr<Fence> VulkanDevice::CreateFence(std::uint64_t initialValue)
    {
        return std::unique_ptr<Fence>(new VulkanFence(this, initialValue));
    }

    std::unique_ptr<StatisticsQuery> VulkanDevice::CreateStatisticsQuery(const std::string& name)
    {
        return std::unique_ptr<StatisticsQuery>(new VulkanStatisticsQuery(this, name));
    }

    std::unique_ptr<TimestampQuery> VulkanDevice::CreateTimestampQuery(std::uint32_t timestampsCount, const std::string& name)
    {
        return std::unique_ptr<TimestampQuery>(new VulkanTimestampQuery(this, timestampsCount, name));
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
        switch (view.GetType())
        {
        case ResourceViewType::CBV:
            CreateBufferCBV(view, descriptor);
            break;
        case ResourceViewType::SRV:
            CreateBufferSRV(view, descriptor);
            break;
        case ResourceViewType::UAV:
            CreateBufferUAV(view, descriptor);
            break;
        default:
            UNREACHABLE("Unsupported buffer view type.");
            break;
        }
    }

    void VulkanDevice::CreateBufferSRV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
        BufferView view(resource.get(),
            ResourceViewType::SRV,
            resource->GetSize(),
            0,
            0,
            resource->GetElementCount(),
            resource->GetStride());

        CreateBufferSRV(view, descriptor);
    }

    void VulkanDevice::CreateBufferCBV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor)
    {
        BufferView view(resource.get(),
            ResourceViewType::CBV,
            resource->GetSize(),
            0,
            0,
            resource->GetElementCount(),
            resource->GetStride());

        CreateBufferCBV(view, descriptor);
    }

    void VulkanDevice::CreateBufferUAV(std::shared_ptr<Buffer> resource, CPUDescriptor& descriptor, std::shared_ptr<Buffer> counterResource)
    {
        BufferView view(resource.get(),
            ResourceViewType::UAV,
            resource->GetSize(),
            0,
            0,
            resource->GetElementCount(),
            resource->GetStride(),
            counterResource.get());

        CreateBufferUAV(view, descriptor);
    }

    void VulkanDevice::CreateTextureView(const TextureView& view, CPUDescriptor& descriptor)
    {
        switch (view.GetType())
        {
        case ResourceViewType::RTV:
            CreateTextureRTV(view, descriptor);
            break;
        case ResourceViewType::DSV:
            CreateTextureDSV(view, descriptor);
            break;
        case ResourceViewType::SRV:
            CreateTextureSRV(view, descriptor);
            break;
        case ResourceViewType::UAV:
            CreateTextureUAV(view, descriptor);
            break;
        default:
            UNREACHABLE("Unsupported texture view type.");
            break;
        }
    }

    void VulkanDevice::CreateTextureRTV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::RTV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureRTV(view, descriptor);
    }

    void VulkanDevice::CreateTextureDSV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::DSV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureDSV(view, descriptor);
    }

    void VulkanDevice::CreateTextureSRV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::SRV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureSRV(view, descriptor);
    }

    void VulkanDevice::CreateTextureUAV(std::shared_ptr<Texture> texture, CPUDescriptor& descriptor)
    {
        TextureView view(texture.get(),
            ResourceViewType::UAV,
            texture->GetDescription().Dimension,
            0,
            texture->GetMipLevels(),
            0,
            0,
            texture->GetDepthOrArraySize());

        CreateTextureUAV(view, descriptor);
    }

    void VulkanDevice::CreateBufferSRV(const BufferView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->WriteBufferDescriptor(static_cast<std::uint32_t>(descriptor.ptr),
            VulkanCast<vk::Buffer>(view.GetBuffer()->GetNative()),
            view.GetOffset(),
            view.GetSize(),
            vk::DescriptorType::eStorageBuffer);
    }

    void VulkanDevice::CreateBufferCBV(const BufferView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->WriteBufferDescriptor(static_cast<std::uint32_t>(descriptor.ptr),
            VulkanCast<vk::Buffer>(view.GetBuffer()->GetNative()),
            view.GetOffset(),
            view.GetSize(),
            vk::DescriptorType::eUniformBuffer);
    }

    void VulkanDevice::CreateBufferUAV(const BufferView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        // A UAV counter needs its own descriptor in Vulkan, not an offset into this one
        heap->WriteBufferDescriptor(static_cast<std::uint32_t>(descriptor.ptr),
            VulkanCast<vk::Buffer>(view.GetBuffer()->GetNative()),
            view.GetOffset(),
            view.GetSize(),
            vk::DescriptorType::eStorageBuffer);
    }

    void VulkanDevice::CreateTextureRTV(const TextureView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->SetImageView(static_cast<std::uint32_t>(descriptor.ptr),
            CreateImageView(view),
            { view.GetTexture()->GetWidth(), view.GetTexture()->GetHeight() });
    }

    void VulkanDevice::CreateTextureDSV(const TextureView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->SetImageView(static_cast<std::uint32_t>(descriptor.ptr),
            CreateImageView(view),
            { view.GetTexture()->GetWidth(), view.GetTexture()->GetHeight() });
    }

    void VulkanDevice::CreateTextureSRV(const TextureView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->WriteImageDescriptor(static_cast<std::uint32_t>(descriptor.ptr),
            CreateImageView(view),
            GetVkDescriptorImageLayout(ResourceViewType::SRV, view.GetFormat()),
            vk::DescriptorType::eSampledImage);
    }

    void VulkanDevice::CreateTextureUAV(const TextureView& view, CPUDescriptor& descriptor)
    {
        auto* heap = static_cast<VulkanDescriptorHeap*>(descriptor.Heap);
        ASSERT(heap, "Descriptor was not issued by a heap.");

        heap->WriteImageDescriptor(static_cast<std::uint32_t>(descriptor.ptr),
            CreateImageView(view),
            GetVkDescriptorImageLayout(ResourceViewType::UAV, view.GetFormat()),
            vk::DescriptorType::eStorageImage);
    }

    vk::ImageView VulkanDevice::CreateImageView(const TextureView& view)
    {
        const std::uint32_t arraySize = view.GetArraySize();

        const vk::ImageViewCreateInfo createInfo =
        {
            .image = VulkanCast<vk::Image>(view.GetTexture()->GetNative()),
            .viewType = GetVkImageViewType(view.GetDimension(), arraySize),
            .format = GetVkFormat(view.GetFormat()),
            .subresourceRange =
            {
                .aspectMask = GetVkImageAspect(view.GetFormat()),
                .baseMipLevel = view.GetMostDetailedMip(),
                .levelCount = view.GetMipLevels() > 0 ? view.GetMipLevels() : vk::RemainingMipLevels,
                .baseArrayLayer = view.GetFirstArraySlice(),
                .layerCount = arraySize > 0 ? arraySize : 1
            }
        };

        auto [result, imageView] = _logicalDevice.createImageView(createInfo);
        VK_CHECK(result, "Failed to create image view");

        return imageView;
    }

    std::uint32_t VulkanDevice::GetDescriptorHandleIncrementSize(DescriptorHeapType) const
    {
        return 1;
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

    SwapChain* VulkanDevice::GetSwapChain() const
    {
        return _swapChain;
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
        if (!_physicalDevice)
        {
            LOG_CRITICAL("No Vulkan physical device supports the required feature set.");
            return nullptr;
        }

        LOG_INFO("Selected Vulkan device: {}", _physicalDevice.getProperties().deviceName.data());

        const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = _physicalDevice.getQueueFamilyProperties();

        const std::uint32_t graphicsQueueFamilyIndex = FindQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eGraphics, {});
        if (graphicsQueueFamilyIndex == kInvalidQueueFamily)
        {
            LOG_CRITICAL("Selected Vulkan device exposes no graphics queue family.");
            return nullptr;
        }

        std::uint32_t computeQueueFamilyIndex = FindQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
        if (computeQueueFamilyIndex == kInvalidQueueFamily)
        {
            computeQueueFamilyIndex = graphicsQueueFamilyIndex;
        }

        std::uint32_t copyQueueFamilyIndex = FindQueueFamilyIndex(queueFamilyProperties, vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
        if (copyQueueFamilyIndex == kInvalidQueueFamily)
        {
            copyQueueFamilyIndex = graphicsQueueFamilyIndex;
        }

        constexpr float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (std::uint32_t queueFamilyIndex : { graphicsQueueFamilyIndex, computeQueueFamilyIndex, copyQueueFamilyIndex })
        {
            const auto isAlreadyRequested = [queueFamilyIndex](const vk::DeviceQueueCreateInfo& info)
            {
                return info.queueFamilyIndex == queueFamilyIndex;
            };

            if (std::ranges::none_of(queueCreateInfos, isAlreadyRequested))
            {
                queueCreateInfos.push_back(
                {
                    .queueFamilyIndex = queueFamilyIndex,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority
                });
            }
        }

        vk::StructureChain<vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
            vk::PhysicalDeviceMutableDescriptorTypeFeaturesEXT>
            featureChain = {
                {.features = {
                    .geometryShader = true,            // Debug draw and point light shadow passes
                    .samplerAnisotropy = true,         // Static samplers s8..s11
                    .pipelineStatisticsQuery = true
                } },
                {.shaderDrawParameters = true},        // Enable shader draw parameters from Vulkan 1.1
                {                                      // Descriptor indexing, required by the bindless heap
                    .descriptorIndexing = true,
                    .shaderSampledImageArrayNonUniformIndexing = true,
                    .shaderStorageBufferArrayNonUniformIndexing = true,
                    .shaderStorageImageArrayNonUniformIndexing = true,
                    .descriptorBindingUniformBufferUpdateAfterBind = true,
                    .descriptorBindingSampledImageUpdateAfterBind = true,
                    .descriptorBindingStorageImageUpdateAfterBind = true,
                    .descriptorBindingStorageBufferUpdateAfterBind = true,
                    .descriptorBindingUpdateUnusedWhilePending = true,
                    .descriptorBindingPartiallyBound = true,
                    .descriptorBindingVariableDescriptorCount = true,
                    .runtimeDescriptorArray = true,
                    .timelineSemaphore = true,
                    .bufferDeviceAddress = true
                },
                {                                      // Vulkan 1.3
                    .synchronization2 = true,          // Sync2 barriers and submits
                    .dynamicRendering = true
                },
                {.extendedDynamicState = true},        // Enable extended dynamic state from the extension
                {.mutableDescriptorType = true}        // One descriptor array holding mixed types
        };

        vk::DeviceCreateInfo deviceCreateInfo = 
        {
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size()),
            .ppEnabledExtensionNames = kDeviceExtensions.data()
        };

        auto [createResult, device] = _physicalDevice.createDevice(deviceCreateInfo);
        VK_CHECK(createResult, "Failed to create Vulkan logical device");

        VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

        _logicalDevice = device;
        _graphicsQueue = std::unique_ptr<VulkanCommandQueue>(new VulkanCommandQueue(this, rhi::CommandListType::Graphics, graphicsQueueFamilyIndex));
        _computeQueue  = std::unique_ptr<VulkanCommandQueue>(new VulkanCommandQueue(this, rhi::CommandListType::Compute, computeQueueFamilyIndex));
        _copyQueue     = std::unique_ptr<VulkanCommandQueue>(new VulkanCommandQueue(this, rhi::CommandListType::Copy, copyQueueFamilyIndex));

        return device;
    }

    VmaAllocator VulkanDevice::CreateAllocator()
    {
        VmaVulkanFunctions functions = {};
        functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
        functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo createInfo = {};
        createInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        createInfo.physicalDevice = _physicalDevice;
        createInfo.device = _logicalDevice;
        createInfo.instance = _instance;
        createInfo.vulkanApiVersion = VK_API_VERSION_1_4;
        createInfo.pVulkanFunctions = &functions;

        VmaAllocator allocator = nullptr;
        VkResult result = vmaCreateAllocator(&createInfo, &allocator);
        VK_CHECK(static_cast<vk::Result>(result), "Failed to create Vulkan memory allocator");

        return allocator;
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
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
            vk::PhysicalDeviceMutableDescriptorTypeFeaturesEXT>();

        const auto& vulkan11 = features.template get<vk::PhysicalDeviceVulkan11Features>();
        const auto& vulkan12 = features.template get<vk::PhysicalDeviceVulkan12Features>();
        const auto& vulkan13 = features.template get<vk::PhysicalDeviceVulkan13Features>();
        const auto& dynamicState = features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        const auto& mutableType = features.template get<vk::PhysicalDeviceMutableDescriptorTypeFeaturesEXT>();
        const auto& core = features.template get<vk::PhysicalDeviceFeatures2>().features;

        const std::array<std::pair<const char*, bool>, 16> required =
        {{
            { "shaderDrawParameters",                          bool(vulkan11.shaderDrawParameters) },
            { "dynamicRendering",                              bool(vulkan13.dynamicRendering) },
            { "synchronization2",                              bool(vulkan13.synchronization2) },
            { "extendedDynamicState",                          bool(dynamicState.extendedDynamicState) },
            { "mutableDescriptorType",                         bool(mutableType.mutableDescriptorType) },
            { "descriptorBindingPartiallyBound",               bool(vulkan12.descriptorBindingPartiallyBound) },
            { "descriptorBindingSampledImageUpdateAfterBind",  bool(vulkan12.descriptorBindingSampledImageUpdateAfterBind) },
            { "descriptorBindingStorageBufferUpdateAfterBind", bool(vulkan12.descriptorBindingStorageBufferUpdateAfterBind) },
            { "descriptorBindingStorageImageUpdateAfterBind",  bool(vulkan12.descriptorBindingStorageImageUpdateAfterBind) },
            { "descriptorBindingVariableDescriptorCount",      bool(vulkan12.descriptorBindingVariableDescriptorCount) },
            { "runtimeDescriptorArray",                        bool(vulkan12.runtimeDescriptorArray) },
            { "timelineSemaphore",                             bool(vulkan12.timelineSemaphore) },
            { "bufferDeviceAddress",                           bool(vulkan12.bufferDeviceAddress) },
            { "pipelineStatisticsQuery",                       bool(core.pipelineStatisticsQuery) },
            { "geometryShader",                                bool(core.geometryShader) },
            { "samplerAnisotropy",                             bool(core.samplerAnisotropy) },
        }};

        bool supportsRequiredFeatures = true;
        for (const auto& [name, supported] : required)
        {
            if (!supported)
            {
                LOG_WARNING("Device '{}' is missing required feature: {}", physicalDevice.getProperties().deviceName.data(), name);
                supportsRequiredFeatures = false;
            }
        }

        return supportsRequiredFeatures;
    }
} // namespace rhi::vulkan
