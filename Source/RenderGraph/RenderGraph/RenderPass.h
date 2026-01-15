#pragma once

#include "RenderGraphResourceId.h"

#include "Renderer/Render/Frame/TaskGPU.h"
#include "Renderer/Helpers/Profiler.h"

#include <functional>

namespace rg
{
    class RenderGraph;
    class RenderPassBuilder;
    class RenderContext;

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

        virtual void PreExecute(RenderContext& context, TaskGPU& task);
        virtual void Execute(RenderContext& context, TaskGPU& task) = 0;
        virtual void PostExecute(RenderContext& context, TaskGPU& task);

        RenderPassType GetType() const;

    protected:
        friend class RenderPassBuilder;
        friend class RenderGraph;

        std::string _name;
        RenderPassType _type;

        std::weak_ptr<IRenderPass> _prevPass;
        std::weak_ptr<IRenderPass> _nextPass;

        std::vector<RGResourceId> _creates;
        std::vector<RGResourceId> _writes;
        std::vector<RGResourceId> _reads;

        std::unordered_map<RGResourceId, dx12::ResourceState> _resourceStateMap;

        std::uint32_t _refCount;

        Profiler::TimerID _gpuTimerID;
    };

    template<typename PassData>
    class RenderPass : public IRenderPass
    {
    public:
        using SetupFunc = std::function<void(RenderPassBuilder&)>;
        using ExecuteFunc = std::function<void(RenderContext&, TaskGPU&)>;

        RenderPass(const std::string& name, RenderPassType type = RenderPassType::Graphics);
        RenderPass(const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute, RenderPassType type = RenderPassType::Graphics);
        RenderPass(const RenderPass&) = delete;
        RenderPass(RenderPass&&) = default;
        ~RenderPass() = default;

        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass& operator=(RenderPass&&) = default;

        const PassData& GetData() const;

        // Inherited via IRenderPass
        void Setup(RenderPassBuilder& builder) override;
        void Execute(RenderContext& context, TaskGPU& task) override;

    protected:
        PassData _data;

        std::function<void(RenderPassBuilder&)> _setupFunc;
        std::function<void(RenderContext&, TaskGPU&)> _executeFunc;
    };
} // namespace rg

#include "RenderPass.inl"
