#pragma once

namespace rhi
{
    class Buffer;
    class Texture;

    // BufferBarrier represents a resource barrier for a buffer resource, which is used to synchronize access to the buffer and ensure proper resource state transitions. 
    // It can be used in command lists to specify the necessary synchronization and state transitions for buffer resources during GPU execution
    class BufferBarrier
    {
    public:
        BufferBarrier(std::shared_ptr<Buffer> targetResource = nullptr, ResourceState beforeState = ResourceState::Common, ResourceState afterState = ResourceState::Common)
            : TargetResource(targetResource), BeforeState(beforeState), AfterState(afterState) 
        {   }

        std::weak_ptr<Buffer> TargetResource;
        ResourceState BeforeState;
        ResourceState AfterState;
    };

    // TextureBarrier represents a resource barrier for a texture resource, which is used to synchronize access to the texture and ensure proper resource state transitions.
    // It can be used in command lists to specify the necessary synchronization and state transitions for texture resources during GPU execution
    class TextureBarrier
    {
    public:
        TextureBarrier(std::shared_ptr<Texture> targetResource = nullptr, ResourceState beforeState = ResourceState::Common, ResourceState afterState = ResourceState::Common)
            : TargetResource(targetResource), BeforeState(beforeState), AfterState(afterState)
        {   }

        std::weak_ptr<Texture> TargetResource;
        ResourceState BeforeState;
        ResourceState AfterState;
    };
} // namespace rhi
