#include "RenderGraphPCH.h"

#include "RenderGraph.h"

#include "RenderPassBuilder.h"

#include "Render/Frame/Frame.h"
#include "Render/Frame/TaskGPU.h"

namespace rg
{
    void RenderGraph::Compile()
    {
        BuildAdjacencyLists();
        TopologicalSort();
    }

    void RenderGraph::Execute(Frame& frame)
    {
        for (auto passIndex : _sortedPasses)
        {
            TaskGPU* task = nullptr;
            switch (_passes[passIndex]->GetType())
            {
            case RenderPassType::Graphics:
                task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
                break;
            case RenderPassType::Compute:
                task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE);
                break;
            case RenderPassType::Copy:
                task = frame.CreateTask(D3D12_COMMAND_LIST_TYPE_COPY);
                break;
            defualt:
                FAIL("Undefined render pass type");
                break;
            }

            task->SetName(_passes[passIndex]->_name);
            _passes[passIndex]->Execute(_context, *task);
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

        _adjacencyLists.resize(passesCount);

        for (size_t passIndex = 0; passIndex < passesCount; ++passIndex)
        {
            std::shared_ptr<IRenderPass>& pass = _passes[passIndex];
            std::vector<std::uint32_t>& passAdjacencyList = _adjacencyLists[passIndex];

            for (size_t i = passIndex + 1; i < passesCount; ++i)
            {
                for (ResourceId readId : _passes[i]->_reads)
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
