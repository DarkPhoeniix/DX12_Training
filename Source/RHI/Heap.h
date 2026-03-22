#pragma once

namespace rhi
{
    class Buffer;
    class Texture;

    struct HeapDescription
    {

    };

    class Heap
    {
    public:
        ~Heap();

        virtual void PlaceResource(Buffer& buffer, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;
        virtual void PlaceResource(Texture& texture, ResourceState state = ResourceState::Common, std::uint64_t offset = (std::uint64_t)-1) = 0;

        virtual void Reset() = 0;

        virtual void* GetNative() const = 0;
    };
} // namespace rhi
