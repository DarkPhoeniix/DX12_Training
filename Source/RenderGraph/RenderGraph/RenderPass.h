#pragma once

#include "Interfaces.h"
#include "RenderGraphResourceId.h"

#include "Renderer/Helpers/Profiler.h"

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

        virtual void PreExecute(RenderContext& context, ITask* task);
        virtual void Execute(RenderContext& context, ITask* task) = 0;
        virtual void PostExecute(RenderContext& context, ITask* task);

        RenderPassType GetType() const;

    protected:
        friend class RenderPassBuilder;
        friend class RenderGraph;

        std::string _name;
        RenderPassType _type;

        std::weak_ptr<IRenderPass> _prevPass;
        std::weak_ptr<IRenderPass> _nextPass;

        std::vector<RGBufferId> _bufferCreates;
        std::vector<RGBufferId> _bufferWrites;
        std::vector<RGBufferId> _bufferReads;

        std::vector<RGTextureId> _textureCreates;
        std::vector<RGTextureId> _textureWrites;
        std::vector<RGTextureId> _textureReads;

        std::vector<RGVirtualResourceId> _virtualWrites;
        std::vector<RGVirtualResourceId> _virtualReads;

        std::unordered_map<RGBufferId, rhi::ResourceState> _bufferStateMap;
        std::unordered_map<RGTextureId, rhi::ResourceState> _textureStateMap;

        std::uint32_t _refCount;

        Profiler::TimerID _gpuTimerID;
    };

    template<typename PassData>
    class RenderPass : public IRenderPass
    {
    public:
        using SetupFunc = std::function<void(RenderPassBuilder&)>;
        using ExecuteFunc = std::function<void(RenderContext&, ITask*)>;

        RenderPass(rhi::Device* device, const std::string& name, RenderPassType type = RenderPassType::Graphics);
        RenderPass(rhi::Device* device, const std::string& name, SetupFunc&& setup, ExecuteFunc&& execute, RenderPassType type = RenderPassType::Graphics);
        RenderPass(const RenderPass&) = delete;
        RenderPass(RenderPass&&) = default;
        ~RenderPass() = default;

        RenderPass& operator=(const RenderPass&) = delete;
        RenderPass& operator=(RenderPass&&) = default;

        const PassData& GetData() const;

        // Inherited via IRenderPass
        void Setup(RenderPassBuilder& builder) override;
        void Execute(RenderContext& context, ITask* task) override;

    protected:
        PassData _data;

        std::function<void(RenderPassBuilder&)> _setupFunc;
        std::function<void(RenderContext&, ITask*)> _executeFunc;

        rhi::Device* _device;
    };
} // namespace rg

#include "RenderPass.inl"
