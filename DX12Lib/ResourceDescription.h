#pragma once

namespace dx12
{
    // Enumeration defining different types of DirectX 12 resources.
    enum class ResourceType : int
    {
        None = 0, // Default uninitialized resource type.

        // Access types.
        Dynamic = 1 << 0, // Resource can be dynamically updated.
        ReadBack = 1 << 1, // Resource is used for reading data back from the GPU.
        Unordered = 1 << 2, // Resource supports unordered access.

        // Resource types.
        Buffer = 1 << 3, // Standard buffer resource.
        Texture = 1 << 4, // Texture resource.
        RenderTarget = 1 << 5, // Render target texture.
        DepthStencil = 1 << 6, // Depth/stencil buffer.

        // Additional flags.
        Aligned = 1 << 7, // Resource is aligned.
        DenyShader = 1 << 8  // Resource is inaccessible by shaders.
    };
    BINARY_OPERATION_TO_ENUM(ResourceType);

    // Wrapper class for describing DirectX 12 resources.
    class ResourceDescription
    {
    public:
        // Default constructor, initializes an empty resource description.
        ResourceDescription();
        // Constructs a resource description from an existing D3D12_RESOURCE_DESC.
        ResourceDescription(const D3D12_RESOURCE_DESC& description);

        // Sets the resource dimension (buffer, texture, etc.).
        void SetDimension(D3D12_RESOURCE_DIMENSION dimension);
        // Retrieves the resource dimension.
        D3D12_RESOURCE_DIMENSION GetDimension() const;

        // Sets the memory alignment of the resource.
        void SetAlignment(std::uint64_t alignment);
        // Retrieves the memory alignment of the resource.
        std::uint64_t GetAlignment() const;

        // Sets the width and height of the resource.
        void SetSize(const DirectX::XMUINT2& size);
        // Retrieves the width and height of the resource.
        DirectX::XMUINT2 GetSize() const;

        // Sets the depth or array size for 3D textures or texture arrays.
        void SetDepthOrArraySize(std::uint16_t depthOrArraySize);
        // Retrieves the depth or array size.
        std::uint16_t GetDepthOrArraySize() const;

        // Sets the number of mip levels for a texture.
        void SetMipLevels(std::uint16_t mipLevels);
        // Retrieves the number of mip levels.
        std::uint16_t GetMipLevels() const;

        // Sets the pixel format of the resource.
        void SetFormat(DXGI_FORMAT format);
        // Retrieves the pixel format.
        DXGI_FORMAT GetFormat() const;

        // Sets the sample description.
        void SetSampleDescription(const DXGI_SAMPLE_DESC& sampleDescription);
        // Retrieves the sample description.
        DXGI_SAMPLE_DESC GetSampleDescription() const;

        // Sets the layout type of the texture.
        void SetLayout(D3D12_TEXTURE_LAYOUT textureLayout);
        // Retrieves the layout type.
        D3D12_TEXTURE_LAYOUT GetLayout() const;

        // Sets the resource flags (e.g., allowing render target usage).
        void SetFlags(D3D12_RESOURCE_FLAGS flags);
        // Adds additional flags to the resource.
        void AddFlags(D3D12_RESOURCE_FLAGS flags);
        // Retrieves the resource flags.
        D3D12_RESOURCE_FLAGS GetFlags() const;

        // Sets the resource type (buffer, texture, etc.).
        void SetResourceType(ResourceType type);
        // Adds an additional resource type flag.
        void AddResourceType(ResourceType type);
        // Retrieves the current resource type.
        ResourceType GetResourceType() const;
        // Checks if the resource has a specific type.
        bool IsType(ResourceType type) const;

        // Sets the data stride for structured buffers.
        void SetStride(std::uint32_t stride);
        // Retrieves the stride.
        std::uint32_t GetStride() const;

        // Sets the clear value for render targets or depth/stencil buffers.
        void SetClearValue(D3D12_CLEAR_VALUE clearValue);
        void SetClearValue(const DirectX::XMFLOAT4& clearValue);
        // Retrieves the clear value.
        std::shared_ptr<D3D12_CLEAR_VALUE> GetClearValue() const;

        // Creates a D3D12_RESOURCE_DESC based on the current settings.
        D3D12_RESOURCE_DESC CreateDXResourceDescription() const;

    private:
        // Updates the resource size based on its type.
        void UpdateSize(ResourceType type);
        // Updates the resource flags based on its type.
        void UpdateFlags(ResourceType type);

        // DirectX resource description structure.
        D3D12_RESOURCE_DESC _resourceDescription;
        // Type of resource (buffer, texture, etc.).
        ResourceType _resourceType;
        // Stride size for structured buffers.
        std::uint32_t _stride;
        // Optional clear value for render targets or depth/stencil buffers.
        std::shared_ptr<D3D12_CLEAR_VALUE> _clearValue;
    };
} // namespace dx12
