#pragma once

namespace rhi
{
    class Buffer;
    class Texture;

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
