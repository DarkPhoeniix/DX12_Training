#pragma once

namespace dx12
{
    class CommandList;

    // TODO: Texture needed?

    // Wrapper for a DirectX 12 texture resource.
    class Texture : public Resource
    {
    public:
        // Copy constructor.
        Texture(const Texture& other) = default;
        // Move constructor.
        Texture(Texture&& other) noexcept = default;
        // Destructor.
        ~Texture() = default;

        // Copy assignment operator.
        Texture& operator=(const Texture& other) = default;
        // Move assignment operator.
        Texture& operator=(Texture&& other) noexcept = default;
    };
} // namespace dx12
