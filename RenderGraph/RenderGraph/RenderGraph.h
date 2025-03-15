#pragma once

#include "RenderPass.h"
#include "RenderContext.h"

#ifdef RG_MULTITHREADED
#include "Helpers/PassWorkerManager.h"
#endif

#include "Scene/Scene.h"

class Frame;
class TaskGPU;

namespace rg
{
    class RenderPassBuilder;

    class RenderGraph
    {
    public:
        RenderGraph();
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) = default;
        ~RenderGraph() = default;

        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph& operator=(RenderGraph&&) = default;

        CacheGPU& GetCache();
        dx12::ResourceTable& GetResourceTable();

        void SetFrame(Frame& frame);

        void Reset();
        void Compile();
        void Execute();

        void AddPass(std::shared_ptr<IRenderPass> pass);

        void ImportResource(std::shared_ptr<dx12::Resource> resource);
        std::shared_ptr<dx12::Resource> ExportResource(const std::string& name);

    private:
        friend class RenderPassBuilder;

        void BuildAdjacencyLists();
        void TopologicalSort();
        
        std::vector<std::vector<std::uint32_t>> _adjacencyLists;
        std::vector<std::shared_ptr<IRenderPass>> _passes;
        std::vector<std::uint32_t> _sortedPasses;
        std::vector<TaskGPU*> _GPUTasks;

        Frame* _frame;
        RenderContext _context;

        std::shared_ptr<scene::Scene> _scene;

#ifdef RG_MULTITHREADED
        mt::PassWorkerManager _workerManager;
#endif
    };
}
