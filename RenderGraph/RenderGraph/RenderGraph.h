#pragma once

#include "RenderPass.h"
#include "RenderContext.h"

class Frame;
class TaskGPU;

namespace rg
{
    class RenderPassBuilder;

    class RenderGraph
    {
    public:
        RenderGraph() = default;
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) = default;
        ~RenderGraph() = default;

        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph& operator=(RenderGraph&&) = default;

        void Reset();
        void Compile();
        void Execute(Frame& frame);

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

        RenderContext _context;
    };
}
