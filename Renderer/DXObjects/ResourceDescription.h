#pragma once

namespace Core
{
    enum class EResourceType : int
    {
        None = 1 << 0,

        // access type 
        Dynamic = 1 << 1,
        ReadBack = 1 << 2,
        Unordered = 1 << 3,

        // type of resource
        Buffer = 1 << 4,
        Texture = 1 << 5,
        RenderTarget = 1 << 6,
        DepthTarget = 1 << 7,

        // addition flags
        StrideAlignment = 1 << 8,

        // acceleration flags
        Deny_shader_resource = 1 << 9,

        Last = 1 << 10
    };
    BINARY_OPERATION_TO_ENUM(EResourceType);

    enum class ETextureType
    {
        None,
        Sampler1D,
        Sampler2D,
        Sampler3D,
        SamplerCube
    };

    class ResourceDescription
    {
    public:
        ResourceDescription();
        ResourceDescription(const D3D12_RESOURCE_DESC& description);

        void SetDimension(D3D12_RESOURCE_DIMENSION dimension);
        D3D12_RESOURCE_DIMENSION GetDimension() const;

        void SetAlignment(UINT64 alignment);
        UINT64 GetAlignment() const;

        void SetSize(const DirectX::XMUINT2& size);
        DirectX::XMUINT2 GetSize() const;

        void SetDepthOrArraySize(UINT16 depthOrArraySize);
        UINT16 getDepthOrArraySize() const;

        void SetMipLevels(UINT16 mipLevels);
        UINT16 GetMipLevels() const;

        void SetFormat(DXGI_FORMAT format);
        DXGI_FORMAT GetFormat() const;

        void SetSampleDescription(const DXGI_SAMPLE_DESC& sampleDescription);
        DXGI_SAMPLE_DESC GetSampleDescription() const;

        void SetLayout(D3D12_TEXTURE_LAYOUT textureLayout);
        D3D12_TEXTURE_LAYOUT GetLayout() const;

        void SetFlags(D3D12_RESOURCE_FLAGS flags);
        void AddFlags(D3D12_RESOURCE_FLAGS flags);
        D3D12_RESOURCE_FLAGS GetFlags() const;

        void SetResourceType(EResourceType type);
        void AddResourceType(EResourceType type);
        EResourceType GetResourceType() const;
        bool IsType(EResourceType type) const;

        void SetStride(UINT64 stride);
        UINT64 GetStride() const;

        void SetClearValue(D3D12_CLEAR_VALUE clearValue);
        void SetClearValue(const DirectX::XMFLOAT4& clearValue);
        std::shared_ptr<D3D12_CLEAR_VALUE> GetClearValue() const;

        D3D12_RESOURCE_DESC CreateDXResourceDescription() const;

    private:
        void UpdateSize(EResourceType type);
        void UpdateFlags(EResourceType type);

        D3D12_RESOURCE_DESC _resourceDescription;
        EResourceType _resourceType;
        UINT64 _stride;
        std::shared_ptr<D3D12_CLEAR_VALUE> _clearValue;
    };
} // namespace Core
