#pragma once

#include "Device.h"
#include "Format.h"
#include "ResourceCommon.h"

namespace rhi
{
    struct ClearValue
    {
        Format Format;
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

    class Texture
    {
    public:
        Texture() = default;
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept = default;
        virtual ~Texture() = default;

        Texture& operator=(const Texture&) = delete;
        Texture& operator=(Texture&&) noexcept = default;

        template<typename Type>
        [[nodiscard]] Type* Map(std::uint32_t begin, std::uint32_t end);
        virtual void* Map(std::uint32_t begin, std::uint32_t end) = 0;
        virtual void Unmap() = 0;

        [[nodiscard]] virtual std::uint64_t GetVirtualAddress() = 0;
        [[nodiscard]] virtual ResourceState GetInitialState() const = 0;
        [[nodiscard]] virtual ResourceState GetCurrentState() const = 0;
        virtual void SetCurrentState(ResourceState state) = 0;

        const TextureDescription& GetDescription() const { return _description; }

        std::uint32_t GetWidth() const { return _description.Width; }
        std::uint32_t GetHeight() const { return _description.Height; }
        std::uint32_t GetMipLevels() const { return _description.MipLevels; }
        std::uint32_t GetDepthOrArraySize() const { return _description.DepthOrArraySize; }
        Format GetFormat() const { return _description.Format; }

        virtual const ResourceID& GetID() const = 0;

        virtual void* GetNative() const = 0;

    protected:
        TextureDescription _description;
    };
} // namespace rhi
