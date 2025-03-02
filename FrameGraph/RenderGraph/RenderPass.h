#pragma once

class TaskGPU;

namespace rg
{
    class RenderPassBuilder;
    class RenderContext;
    class RenderGraph;

    using ResourceId = std::uint64_t;

    enum class RenderPassType
    {
        Graphics,
        Compute,
        Copy
    };

    class IRenderPass
    {
    public:
        IRenderPass(const std::string& name, RenderPassType type);
        IRenderPass(const IRenderPass&) = delete;
        IRenderPass(IRenderPass&&) = default;
        virtual ~IRenderPass() = default;

        IRenderPass& operator=(const IRenderPass&) = delete;
        IRenderPass& operator=(IRenderPass&&) = default;

        virtual void Setup(RenderPassBuilder& builder) = 0;
        virtual void Execute(RenderContext& context, TaskGPU& task) = 0;

        RenderPassType GetType() const;

    protected:
        friend class RenderPassBuilder;
        friend class RenderGraph;

        std::vector<ResourceId> _creates;
        std::vector<ResourceId> _reads;
        std::vector<ResourceId> _writes;

        std::uint32_t _refCount;

        RenderPassType _type;
        std::string _name;
    };

    template<typename PassData>
    class RenderPass : public IRenderPass
    {
    public:
        RenderPass(const std::string& name, RenderPassType type = RenderPassType::Graphics);
        RenderPass(const RenderPass&) = delete;
        RenderPass(RenderPass&&) = default;
        ~RenderPass() = default;

        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass& operator=(RenderPass&&) = default;

        const PassData& GetData() const;

    protected:
        PassData _data;
    };
}

#include "RenderPass.inl"
