#pragma once

#include "Interfaces.h"
#include "RenderPass.h"
#include "RenderContext.h"

#include "Helpers/PassWorkerManager.h"

#include "Renderer/Helpers/Profiler.h"

namespace rg
{
    class RenderPassBuilder;

    class RenderGraph
    {
    public:
        RenderGraph(rhi::Device* device, IDescriptorProvider* decriptorProvider);
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) = default;
        ~RenderGraph() = default;

        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph& operator=(RenderGraph&&) = default;

        void SetTaskAllocator(ITaskAllocator* allocator);
        void SetFrameBuffer(rhi::Buffer* buffer);

        void Reset();
        void Compile();
        void Execute();

        template<typename PassData, typename ...Args> requires std::is_constructible_v<RenderPass<PassData>, Args...>
        void AddPass(Args&& ...args)
        {   
            auto pass = std::make_shared<RenderPass<PassData>>(std::forward<Args>(args)...);
            AddPass(pass);
        }
        void AddPass(std::shared_ptr<IRenderPass> pass);

        void ImportResource(std::shared_ptr<rhi::Buffer> resource, const std::string& name);
        void ImportResource(std::shared_ptr<rhi::Texture> resource, const std::string& name);
        void ExportResource(const std::string& name, std::shared_ptr<rhi::Texture> desctination);
        void ExportResource(const std::string& name, std::shared_ptr<rhi::Buffer> desctination);

#if ENABLE_PROFILING
        void SetGPUProfiler(Profiler* gpuProfiler);
        Profiler* GetGPUProfiler() const { return _gpuProfiler; }
#endif

    private:
        friend class RenderPassBuilder;

        void BeginFrame();
        void EndFrame();

        void BuildAdjacencyLists();
        void TopologicalSort();
        void TransitionResourcesToWorkingState();

        std::vector<std::vector<std::uint32_t>> _adjacencyLists;
        std::vector<std::shared_ptr<IRenderPass>> _passes;
        std::vector<std::uint32_t> _sortedPasses;
        std::vector<ITask*> _GPUTasks;

#if ENABLE_PROFILING
        Profiler* _gpuProfiler;

        ITask* _beginFrameTask;
        ITask* _endFrameTask;
        Profiler::TimerID _frameTimerID;
#endif // ENABLE_PROFILING

        bool _transitionedToWorkingState = false;

        ITaskAllocator* _taskAllocator;
        RenderContext _context;

        std::unique_ptr<mt::PassWorkerManager> _workerManager;
    };
} // namespace rg
