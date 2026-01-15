#include "RenderGraphPCH.h"

#include "RenderGraph.h"

#include "RenderPassBuilder.h"

#include "RHI/ResourceBarrier.h"
#include "Renderer/Render/Frame/Frame.h"
#include "Renderer/Render/Frame/TaskGPU.h"

namespace
{
    static constexpr std::uint32_t RENDER_THREADS_NUM = 4;
    static constexpr std::uint32_t TASKS_PER_PASS = 3;
}

namespace rg
{
    RenderGraph::RenderGraph()
        : _frame(nullptr)
        , _context()
#ifdef RG_MULTITHREADED
        , _workerManager(std::make_unique<mt::PassWorkerManager>(RENDER_THREADS_NUM))
#else
        , _workerManager(nullptr)
#endif
#if ENABLE_PROFILING
        , _gpuProfiler(nullptr)
#endif
    {
    }

    void RenderGraph::SetFrame(Frame& frame)
    {
        _frame = &frame;
        _context._frame = &frame;
    }

    void RenderGraph::Init(ResourceTable& resourceTable, TextureManager& textureManager)
    {
		_context.Init(resourceTable, textureManager);
    }

    void RenderGraph::Reset()
    {
#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            _frameTimerID = _gpuProfiler->RegisterTimer("Frame");
        }
#endif

        _context._mapNameToId.clear();
        _context._mapIdToResource.clear();

        _passes.clear();
        _sortedPasses.clear();
    }

    void RenderGraph::Compile()
    {
        _sortedPasses.clear();

        BuildAdjacencyLists();
        TopologicalSort();

        _transitionedToWorkingState = false;

        LOG_INFO("Render graph compiled successfully with {} passes.", _passes.size());
    }

    void RenderGraph::Execute()
    {
        BeginFrame();

        _GPUTasks.clear();
        _GPUTasks.resize(_passes.size() * TASKS_PER_PASS, nullptr);

        for (std::uint32_t passIndex : _sortedPasses)
        {
            std::shared_ptr<IRenderPass> pass = _passes[passIndex];

            TaskGPU* preExecutionTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
            preExecutionTask->SetName(std::format("{} - PreExecute task", pass->_name));

            TaskGPU* executionTask = nullptr;
            switch (pass->GetType())
            {
            case RenderPassType::Graphics:
                executionTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
                break;
            case RenderPassType::Compute:
                executionTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COMPUTE);
                break;
            case RenderPassType::Copy:
                executionTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_COPY);
                break;
            }
            executionTask->SetName(std::format("{} - Execute task", pass->_name));

            TaskGPU* postExecutionTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
            postExecutionTask->SetName(std::format("{} - PostExecute task", pass->_name));

            if (executionTask)
            {
                executionTask->SetName(pass->_name);

#ifdef RG_MULTITHREADED
                _workerManager->Submit({ pass.get(), &_context, preExecutionTask, executionTask, postExecutionTask });
#else
                pass->PreExecute(_context, *preExecutionTask);
                pass->Execute(_context, *executionTask);
                pass->PostExecute(_context, *postExecutionTask);
#endif
            }

            _GPUTasks[passIndex * TASKS_PER_PASS] = preExecutionTask;
            _GPUTasks[passIndex * TASKS_PER_PASS + 1] = executionTask;
            _GPUTasks[passIndex * TASKS_PER_PASS + 2] = postExecutionTask;
        }

#ifdef RG_MULTITHREADED
        _workerManager->Wait();
#endif

        EndFrame();
    }

    void RenderGraph::AddPass(std::shared_ptr<IRenderPass> pass)
    {
        ASSERT(pass, "Trying to add a null render pass to the render graph.");
        _passes.push_back(pass);

#if ENABLE_PROFILING
        Profiler::TimerID timerID = Profiler::InvalidTimerID;
        if (_gpuProfiler)
        {
            timerID = _gpuProfiler->RegisterTimer(pass->_name);
        }
        pass->_gpuTimerID = timerID;
#endif

        RenderPassBuilder builder(*this, pass.get());
        _passes.back()->Setup(builder);
    }

    void RenderGraph::ImportResource(std::shared_ptr<dx12::Resource> resource)
    {
        ASSERT(resource, "Trying to import a null resource into the render graph.");

        auto it = _context._mapNameToId.find(resource->GetName());
        if (it != _context._mapNameToId.end())
        {
            _context._mapIdToResource[it->second] = resource;
        }
        else
        {
            const RGResourceId& id = resource->GetID();
            _context._mapNameToId[resource->GetName()] = id;
            _context._mapIdToResource[id] = resource;
        }
    }

    void RenderGraph::ExportResource(const std::string& name, std::shared_ptr<dx12::Resource> destination)
    {
        ASSERT(destination, "Trying to export a resource to a null destination.");

        struct ExportPassData
        {
            RGResourceId SourceId;
        } data;


        AddPass<ExportPassData>("Export Pass",
            [&](RenderPassBuilder& builder)
            {
                data.SourceId = builder.CopySrcTexture(name);
            },
            [&](RenderContext& context, TaskGPU& task)
            {
                task.SetName("Export Pass");
                dx12::CommandList& commandList = task.GetCommandList();

                std::shared_ptr<dx12::Resource> source = context.GetResource(data.SourceId);
                commandList.CopyResource(*destination, *source);
                commandList.Close();
            }, 
            RenderPassType::Copy);
    }

#if ENABLE_PROFILING
    void RenderGraph::SetGPUProfiler(Profiler* gpuProfiler)
    {
        _gpuProfiler = gpuProfiler;
        _context.SetGPUProfiler(gpuProfiler);
    }
#endif

    void RenderGraph::BeginFrame()
    {
#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            _beginFrameTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
            _beginFrameTask->SetName("Begin Frame Task");
            dx12::CommandList& commandList = _beginFrameTask->GetCommandList();

            _gpuProfiler->BeginEvent(commandList, _frameTimerID);

            commandList.Close();
        }
#endif

        if (!_transitionedToWorkingState)
        {
            TransitionResourcesToWorkingState();
            _transitionedToWorkingState = true;
        }
    }

    void RenderGraph::EndFrame()
    {
#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            _endFrameTask = _frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
            _endFrameTask->SetName("End Frame Task");
            dx12::CommandList& commandList = _endFrameTask->GetCommandList();

            _gpuProfiler->EndEvent(commandList, _frameTimerID);
            _gpuProfiler->ResolveTimestamps(commandList);

            commandList.Close();
        }
#endif

        for (std::uint32_t passIndex : _sortedPasses)
        {
            TaskGPU* currentPreTask = _GPUTasks[passIndex * TASKS_PER_PASS];
            TaskGPU* currentTask = _GPUTasks[passIndex * TASKS_PER_PASS + 1];
            TaskGPU* currentPostTask = _GPUTasks[passIndex * TASKS_PER_PASS + 2];

            for (std::uint32_t adjacentPassIndex : _adjacencyLists[passIndex])
            {
                TaskGPU* dependentTask = _GPUTasks[adjacentPassIndex * TASKS_PER_PASS];
                dependentTask->AddDependency(currentTask->GetName());
            }

            currentTask->AddDependency(currentPreTask->GetName());
            currentPostTask->AddDependency(currentTask->GetName());
        }

#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            TaskGPU* firstTask = _GPUTasks[_sortedPasses.front() * TASKS_PER_PASS];
            TaskGPU* preLastTask = _GPUTasks[(_sortedPasses.back() - 1) * TASKS_PER_PASS + 2];
            TaskGPU* lastTask = _GPUTasks[_sortedPasses.back() * TASKS_PER_PASS];

            firstTask->AddDependency(_beginFrameTask->GetName());
            _endFrameTask->AddDependency(lastTask->GetName());
        }
#endif
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
                for (RGResourceId readId : otherPass->_reads)
                {
                    const std::vector<rg::RGResourceId>& passWriteIds = pass->_writes;
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

        // Set previous and next pass pointers
        for (size_t i = 0; i < _sortedPasses.size() - 1; ++i)
        {
            std::shared_ptr<IRenderPass> currentPass = _passes[_sortedPasses[i]];
            std::shared_ptr<IRenderPass> nextPass = _passes[_sortedPasses[i + 1]];

            currentPass->_nextPass = nextPass;
            nextPass->_prevPass = currentPass;
        }
    }

    void RenderGraph::TransitionResourcesToWorkingState()
    {
        std::vector<dx12::ResourceBarrier> barriers;

        for (std::shared_ptr<IRenderPass> renderPass : _passes)
        {
            if (!renderPass->_creates.empty())
            {
                for (auto id : renderPass->_creates)
                {
                    std::shared_ptr<dx12::Resource> resource = _context.GetResource(id);
                    if (!resource)
                    {
                        continue;
                    }

                    std::shared_ptr<IRenderPass> lastPass = nullptr;
                    std::shared_ptr<IRenderPass> currentPass = renderPass;

                    while (currentPass)
                    {
                        if (currentPass->_resourceStateMap.find(id) != currentPass->_resourceStateMap.end())
                        {
                            lastPass = currentPass;
                        }

                        currentPass = currentPass->_nextPass.lock();
                    }

                    if (lastPass && lastPass->_resourceStateMap.find(id) != lastPass->_resourceStateMap.end())
                    {
                        dx12::ResourceState lastState = lastPass->_resourceStateMap[id];
                        dx12::ResourceState currState = resource->GetCurrentState();
                        if (lastState != currState)
                        {
                            barriers.push_back({ resource, currState, lastState });
                        }
                    }
                }
            }

            if (!renderPass->_reads.empty())
            {
                for (auto readId : renderPass->_reads)
                {
                    if (std::find(renderPass->_creates.cbegin(), renderPass->_creates.cend(), readId) != renderPass->_creates.cend())
                    {
                        continue;
                    }

                    int refCount = 1;

                    for (std::shared_ptr<IRenderPass> p : _passes)
                    {
                        if (p != renderPass && p->_resourceStateMap.find(readId) != p->_resourceStateMap.end())
                        {
                            refCount++;
                            break;
                        }
                    }

                    if (refCount <= 1)
                    {
                        std::shared_ptr<dx12::Resource> resource = _context.GetResource(readId);

                        if (resource)
                        {
                            dx12::ResourceState lastState = renderPass->_resourceStateMap[readId];
                            dx12::ResourceState currState = resource->GetCurrentState();
                            if (lastState != currState)
                            {
                                barriers.push_back({ resource, currState, lastState });
                            }
                        }
                    }
                }
            }
        }

        TaskGPU& task = *_frame->CreateTask(D3D12_COMMAND_LIST_TYPE_DIRECT);
        task.SetName("Resource State Transition");

        dx12::CommandList& commandList = task.GetCommandList();

        commandList.TransitionBarriers(barriers);
        commandList.Close();
    }
} // namespace rg
