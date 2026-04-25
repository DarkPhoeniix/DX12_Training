#pragma once

#include "Device.h"
#include "Format.h"
#include "ResourceCommon.h"

namespace rhi
{
    // ClearValue represents the clear value for a texture resource
    struct ClearValue
    {
        union
        {
            struct
            {
                float R = 0.0f;
                float G = 0.0f;
                float B = 0.0f;
                float A = 0.0f;
            } Color;
            struct
            {
                float Depth = 0.0f;
                std::uint8_t Stencil = 0;
            } DepthStencil;
        };
    };

    // TextureDescription encapsulates the properties and configuration of a GPU texture resource
    struct TextureDescription
    {
        std::uint32_t Width = 0;
        std::uint32_t Height = 0;
        std::uint16_t DepthOrArraySize = 1;
        std::uint16_t MipLevels = 1;

        ClearValue ClearValue = {};
        ResourceState InitialState = ResourceState::Common;

        Format Format = Format::UNKNOWN;
        TextureDimension Dimension = TextureDimension::Unknown;
        ResourceUsage Usage = ResourceUsage::Default;
        ResourceFlags Flags = ResourceFlags::None;
    };

    // Texture is an abstract interface representing a GPU texture resource, which can be used for various purposes such as render targets, shader resources, or depth-stencil buffers
    class Texture
    {
    public:
        Texture() = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept = default;
        virtual ~Texture() = default;

        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) noexcept = default;

        // Maps the texture resource to CPU accessible memory, allowing the application to read or write data to the texture. 
        // The begin and end parameters specify the range of the texture to map, with default values indicating the entire texture. 
        // The template version of the Map function provides a convenient way to map the texture and obtain a typed pointer to the mapped memory
        template<typename Type>
        [[nodiscard]] Type* Map(std::uint32_t begin, std::uint32_t end);
        // Maps the texture resource to CPU accessible memory, allowing the application to read or write data to the texture.
        // The begin and end parameters specify the range of the texture to map, with default values indicating the entire texture
        virtual void* Map(std::uint32_t begin, std::uint32_t end) = 0;
        // Unmaps the texture resource, indicating that the application has finished accessing the mapped memory and allowing the GPU to use the texture again
        virtual void Unmap() = 0;

        // Retrieves the virtual address of the texture resource, which can be used for GPU access and binding
        [[nodiscard]] virtual std::uint64_t GetVirtualAddress() = 0;
        // Retrieves the initial resource state of the texture, which indicates how the texture is expected to be used when it is first created
        [[nodiscard]] virtual ResourceState GetInitialState() const = 0;
        // Retrieves the current resource state of the texture, which indicates how the texture is currently being used and may differ from the initial state
        [[nodiscard]] virtual ResourceState GetCurrentState() const = 0;
        // Sets the current resource state of the texture, allowing the application to track and manage resource state transitions for proper synchronization and usage
        virtual void SetCurrentState(ResourceState state) = 0;

        // Retrieves the description of the texture
        virtual const TextureDescription& GetDescription() const = 0;

        // Retrieves the width of the texture, which is the number of pixels in the horizontal dimension
        virtual std::uint32_t GetWidth() const = 0;
        // Retrieves the height of the texture, which is the number of pixels in the vertical dimension
        virtual std::uint32_t GetHeight() const = 0;
        // Retrieves the number of mip levels in the texture, which indicates how many levels of detail are available for the texture
        virtual std::uint32_t GetMipLevels() const = 0;
        // Retrieves the depth or array size of the texture, which indicates the number of layers in a texture array or the depth of a 3D texture
        virtual std::uint32_t GetDepthOrArraySize() = 0;
        // Retrieves the format of the texture, which specifies the data layout and type of each texel
        virtual Format GetFormat() const = 0;
        // Retrieves the dimension of the texture
        virtual TextureDimension GetDimension() const = 0;

        // Retrieves the unique identifier of the texture resource.
        // ID unique across all resources (both buffers and textures) created by the device
        virtual const ResourceID& GetID() const = 0;

        // Retrieves a pointer to the native texture object
        virtual void* GetNative() const = 0;
    };
} // namespace rhi
