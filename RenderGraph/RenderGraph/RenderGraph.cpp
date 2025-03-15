#include "RenderGraphPCH.h"

#include "RenderGraph.h"

#include "RenderPassBuilder.h"

#include "Render/Frame/Frame.h"
#include "Render/Frame/TaskGPU.h"

namespace
{
    static constexpr std::uint32_t RENDER_THREADS_NUM = 4;
}

namespace rg
{
    RenderGraph::RenderGraph()
        : _frame(nullptr)
#ifdef RG_MULTITHREADED
        , _workerManager(RENDER_THREADS_NUM)
#endif
    {
    }

    CacheGPU& RenderGraph::GetCache()
    {
        return _context.GetCache();
    }

    dx12::ResourceTable& RenderGraph::GetResourceTable()
    {
        return _context.GetResourceTable();
    }

    void RenderGraph::SetFrame(Frame& frame)
    {
        _frame = &frame;
        _context._currentFrameIndex = frame.Index;
    }

    void RenderGraph::Reset()
    {
        _context._mapNameToId.clear();
        _context._resources.clear();

        _passes.clear();
        _sortedPasses.clear();

        for (auto& cache : _context._cache)
        {
            cache.Clear();
        }
        for (auto& table : _context._resourceTable)
        {
            table.Reset();
        }
    }

    void RenderGraph::Compile()
    {
        _sortedPasses.clear();

        BuildAdjacencyLists();
        TopologicalSort();
    }

    void RenderGraph::Execute()
    {
        _GPUTasks.clear();
        _GPUTasks.resize(_passes.size(), nullptr);

        for (auto passIndex : _sortedPasses)
        {
            std::shared_ptr<IRenderPass> pass = _passes[passIndex];

            TaskGPU* task = nullptr;
            switch (pass->GetType())
            {
            case RenderPassType::Graphics:
                task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
                break;
            case RenderPassType::Compute:
                task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE);
                break;
            case RenderPassType::Copy:
                task = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COPY);
                break;
            defualt:
                FAIL("Undefined render pass type");
                break;
            }

            if (task)
            {
                task->SetName(pass->_name);

#ifdef RG_MULTITHREADED
                _workerManager.Submit({ pass.get(), &_context, task});
#else
                pass->Execute(_context, *task);
#endif
            }

            _GPUTasks[passIndex] = task;
        }

#ifdef RG_MULTITHREADED
        _workerManager.Wait();
#endif

        for (auto passIndex : _sortedPasses)
        {
            TaskGPU* currentTask = _GPUTasks[passIndex];

            for (auto adjacentPassIndex : _adjacencyLists[passIndex])
            {
                TaskGPU* dependentTask = _GPUTasks[adjacentPassIndex];
                dependentTask->AddDependency(currentTask->GetName());
                break;
            }
        }
    }

    void RenderGraph::AddPass(std::shared_ptr<IRenderPass> pass)
    {
        _passes.push_back(pass);

        RenderPassBuilder builder(*this, pass.get());
        _passes.back()->Setup(builder);
    }

    void RenderGraph::ImportResource(std::shared_ptr<dx12::Resource> resource)
    {
        ResourceId id = _context._mapNameToId.size();

        _context._mapNameToId[resource->GetName()] = id;
        _context._resources[id] = resource;
    }

    std::shared_ptr<dx12::Resource> RenderGraph::ExportResource(const std::string& name)
    {
        ResourceId id = _context._mapNameToId[name];

        return _context._resources[id];
    }

    void RenderGraph::BuildAdjacencyLists()
    {
        size_t passesCount = _passes.size();

        _adjacencyLists.clear();
        _adjacencyLists.resize(passesCount);

        for (size_t passIndex = 0; passIndex < passesCount; ++passIndex)
        {
            std::shared_ptr<IRenderPass>& pass = _passes[passIndex];
            std::vector<std::uint32_t>& passAdjacencyList = _adjacencyLists[passIndex];

            for (size_t i = passIndex + 1; i < passesCount; ++i)
            {
                std::shared_ptr<IRenderPass>& otherPass = _passes[i];
                for (ResourceId readId : otherPass->_reads)
                {
                    const auto& passWriteIds = pass->_writes;
                    if (std::find(passWriteIds.cbegin(), passWriteIds.cend(), readId) != passWriteIds.cend())
                    {
                        passAdjacencyList.push_back(i);
                        break;
                    }
                }
            }
        }
    }

    void RenderGraph::TopologicalSort()
    {
        std::vector<bool> visited(_passes.size(), false);

        std::function<void(size_t)> DFS = [&](std::uint32_t passIndex)
            {
                visited[passIndex] = true;
                for (std::uint32_t j : _adjacencyLists[passIndex])
                {
                    if (!visited[j])
                    {
                        DFS(j);
                    }
                }

                _sortedPasses.push_back(passIndex);
            };

        for (std::uint32_t i = 0; i < _passes.size(); i++)
        {
            if (visited[i] == false) 
            {
                DFS(i);
            }
        }

        std::reverse(_sortedPasses.begin(), _sortedPasses.end());
    }
}
