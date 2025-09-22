#pragma once

#include "RenderPass.h"
#include "RenderContext.h"

#include "Helpers/PassWorkerManager.h"

#include "Core/ResourceTable.h"
#include "Core/TextureManager.h"
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

        void SetFrame(Frame& frame);

        void Init(ResourceTable& resourceTable, TextureManager& textureManager);

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

        void ImportResource(std::shared_ptr<dx12::Resource> resource);
        void ExportResource(const std::string& name, std::shared_ptr<dx12::Resource> desctination);

    private:
        friend class RenderPassBuilder;

        void BuildAdjacencyLists();
        void TopologicalSort();
        void TransitionResourcesToWorkingState();

        std::vector<std::vector<std::uint32_t>> _adjacencyLists;
        std::vector<std::shared_ptr<IRenderPass>> _passes;
        std::vector<std::uint32_t> _sortedPasses;
        std::vector<TaskGPU*> _GPUTasks;

        bool _transitionedToWorkingState = false;

        Frame* _frame;
        RenderContext _context;

        std::shared_ptr<scene::Scene> _scene;

        std::unique_ptr<mt::PassWorkerManager> _workerManager;
    };
} // namespace rg
