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

    template<RGResourceType ResourceType>
    struct TypedRGResourceId
    {
        using ResourceId = std::uint64_t;
        static constexpr ResourceId InvalidID = ResourceId(-1);

        TypedRGResourceId() = default;
        TypedRGResourceId(std::uint64_t id)
            : ID(id)
        {   }
        TypedRGResourceId(const TypedRGResourceId&) = default;
        ~TypedRGResourceId() = default;

        auto operator<=>(const TypedRGResourceId&) const = default;

        void Invalidate() { ID = InvalidID; }
        bool IsValid() const { return ID != InvalidID; }

        ResourceId ID;
    };

    using RGBufferId = TypedRGResourceId<RGResourceType::Buffer>;
    using RGTextureId = TypedRGResourceId<RGResourceType::Texture>;
    using RGVirtualResourceId = TypedRGResourceId<RGResourceType::Virtual>;

    template<RGResourceMode Mode>
    struct RGBufferModeId : public RGBufferId
    {
        using RGBufferId::RGBufferId;

        RGBufferModeId(RGBufferId id) : RGBufferId(id) {}
    };

    template<RGResourceMode Mode>
    struct RGTextureModeId : public RGTextureId
    {
        using RGTextureId::RGTextureId;

        RGTextureModeId(RGTextureId id) : RGTextureId(id) {}
    };

    template<RGResourceMode Mode>
    struct RGVirtualResourceModeId : public RGVirtualResourceId
    {
        using RGVirtualResourceId::RGVirtualResourceId;

        RGVirtualResourceModeId(RGTextureId id) : RGVirtualResourceId(id) {}
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
    template<rg::RGResourceType ResourceType>
    struct hash<rg::TypedRGResourceId<ResourceType>>
    {
        std::size_t operator()(const rg::TypedRGResourceId<ResourceType>& id) const noexcept
        {
            return std::hash<rg::TypedRGResourceId<ResourceType>::ResourceId>()(id.ID);
        }
    };
}
