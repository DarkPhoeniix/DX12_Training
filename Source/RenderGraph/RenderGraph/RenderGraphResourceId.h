#pragma once

namespace rg
{
    enum class RGResourceType
    {
        Buffer,
        Texture,
        Virtual
    };

    enum class RGResourceMode
    {
        Read,
        Write,
        Upload,
        CopySrc,
        CopyDst,
        IndirectArgs,
        Vertex,
        Index,
        Constant,
        RenderTarget,
        DepthStencilRead,
        DepthStencilWrite,
        ShaderResource,
        UnorderedAccess
    };

    struct RGResourceId
    {
        using ResourceId = std::uint64_t;
        static constexpr ResourceId InvalidID = ResourceId(-1);

        RGResourceId();
        RGResourceId(std::uint64_t id);
        RGResourceId(const RGResourceId&) = default;
        ~RGResourceId() = default;

        auto operator<=>(const RGResourceId&) const = default;

        void Invalidate();
        bool IsValid() const;

        ResourceId ID;
    };

    template<RGResourceType ResourceType>
    struct TypedRGResourceId : public RGResourceId
    {
        using RGResourceId::RGResourceId;

        TypedRGResourceId(RGResourceId id) : RGResourceId(id) {}
    };

    using RGBufferId = TypedRGResourceId<RGResourceType::Buffer>;
    using RGTextureId = TypedRGResourceId<RGResourceType::Texture>;
    using RGVirtualResourceId = TypedRGResourceId<RGResourceType::Virtual>;

    template<RGResourceMode Mode>
    struct RGBufferModeId : public RGBufferId
    {
        using RGBufferId::RGBufferId;

        RGBufferModeId(RGResourceId id) : RGBufferId(id) {}
    };

    template<RGResourceMode Mode>
    struct RGTextureModeId : public RGTextureId
    {
        using RGTextureId::RGTextureId;

        RGTextureModeId(RGResourceId id) : RGTextureId(id) {}
    };

    template<RGResourceMode Mode>
    struct RGVirtualResourceModeId : public RGVirtualResourceId
    {
        using RGVirtualResourceId::RGVirtualResourceId;

        RGVirtualResourceModeId(RGResourceId id) : RGVirtualResourceId(id) {}
    };

    using RGBufferReadId = RGBufferModeId<RGResourceMode::Read>;
    using RGBufferWriteId = RGBufferModeId<RGResourceMode::Write>;
    using RGBufferUploadId = RGBufferModeId<RGResourceMode::Upload>;
    using RGBufferCopySrcId = RGBufferModeId<RGResourceMode::CopySrc>;
    using RGBufferCopyDstId = RGBufferModeId<RGResourceMode::CopyDst>;
    using RGBufferIndirectArgsId = RGBufferModeId<RGResourceMode::IndirectArgs>;

    using RGTextureReadId = RGTextureModeId<RGResourceMode::Read>;
    using RGTextureWriteId = RGTextureModeId<RGResourceMode::Write>;
    using RGTextureCopySrcId = RGTextureModeId<RGResourceMode::CopySrc>;
    using RGTextureCopyDstId = RGTextureModeId<RGResourceMode::CopyDst>;
    using RGTextureRenderTargetId = RGTextureModeId<RGResourceMode::RenderTarget>;
    using RGTextureDepthStencilReadId = RGTextureModeId<RGResourceMode::DepthStencilRead>;
    using RGTextureDepthStencilWriteId = RGTextureModeId<RGResourceMode::DepthStencilWrite>;

    using RGVirtualResourceReadId = RGVirtualResourceModeId<RGResourceMode::Read>;
    using RGVirtualResourceWriteId = RGVirtualResourceModeId<RGResourceMode::Write>;
} // namespace rg

namespace std
{
    template<>
    struct hash<rg::RGResourceId>
    {
        std::size_t operator()(const rg::RGResourceId& id) const noexcept
        {
            return std::hash<rg::RGResourceId::ResourceId>()(id.ID);
        }
    };
}
