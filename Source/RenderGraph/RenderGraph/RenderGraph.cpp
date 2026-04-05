#include "RenderGraphPCH.h"

#include "RenderGraph.h"

#include "RenderPassBuilder.h"

#include "RHI/ResourceBarrier.h"

namespace
{
    static constexpr std::uint32_t RENDER_THREADS_NUM = 4;
    static constexpr std::uint32_t TASKS_PER_PASS = 3;
}

namespace rg
{
    RenderGraph::RenderGraph(rhi::Device* device, IDescriptorProvider* decriptorProvider)
        : _taskAllocator(nullptr)
        , _context(device, decriptorProvider)
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

    void RenderGraph::SetTaskAllocator(ITaskAllocator& allocator)
    {
        _taskAllocator = &allocator;
    }

    void RenderGraph::Reset()
    {
#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            _frameTimerID = _gpuProfiler->RegisterTimer("Frame");
        }
#endif

        _context._mapNameToBufferId.clear();
        _context._mapNameToTextureId.clear();
        _context._mapIdToBuffer.clear();
        _context._mapIdToTexture.clear();

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

            ITask* preExecutionTask = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
            preExecutionTask->SetName(std::format("{} - PreExecute task", pass->_name));

            ITask* executionTask = nullptr;
            switch (pass->GetType())
            {
            case RenderPassType::Graphics:
                executionTask = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
                break;
            case RenderPassType::Compute:
                executionTask = _taskAllocator->AllocateTask(rhi::CommandListType::Compute);
                break;
            case RenderPassType::Copy:
                executionTask = _taskAllocator->AllocateTask(rhi::CommandListType::Copy);
                break;
            }
            executionTask->SetName(std::format("{} - Execute task", pass->_name));

            ITask* postExecutionTask = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
            postExecutionTask->SetName(std::format("{} - PostExecute task", pass->_name));

            if (executionTask)
            {
                executionTask->SetName(pass->_name);

#ifdef RG_MULTITHREADED
                _workerManager->Submit({ pass.get(), &_context, preExecutionTask, executionTask, postExecutionTask });
#else
                pass->PreExecute(_context, preExecutionTask);
                pass->Execute(_context, executionTask);
                pass->PostExecute(_context, postExecutionTask);
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

    void RenderGraph::ImportResource(std::shared_ptr<rhi::Texture> resource, const std::string& name)
    {
        ASSERT(resource, "Trying to import a null resource into the render graph.");

        auto it = _context._mapNameToTextureId.find(name);
        if (it != _context._mapNameToTextureId.end())
        {
            _context._mapIdToTexture[it->second] = resource;
        }
        else
        {
            const RGTextureId& id = resource->GetID();
            _context._mapNameToTextureId[name] = id;
            _context._mapIdToTexture[id] = resource;
        }
    }

    void RenderGraph::ExportResource(const std::string& name, std::shared_ptr<rhi::Texture> destination)
    {
        ASSERT(destination, "Trying to export a resource to a null destination.");

        struct ExportPassData
        {
            RGTextureId SourceId;
        } data;

        NOT_IMPLEMENTED();
        //AddPass<ExportPassData>("Export Pass",
        //    [&](RenderPassBuilder& builder)
        //    {
        //        data.SourceId = builder.CopySrcTexture(name);
        //    },
        //    [&](RenderContext& context, ITask* task)
        //    {
        //        task->SetName("Export Pass");
        //        rhi::CommandList* commandList = task->GetCommandList();

        //        std::shared_ptr<rhi::Texture> source = context.GetTexture(data.SourceId);
        //        commandList->CopyTexture(destination, source);
        //        commandList->Close();
        //    }, 
        //    RenderPassType::Copy);
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
            _beginFrameTask = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
            _beginFrameTask->SetName("Begin Frame Task");
            rhi::CommandList* commandList = _beginFrameTask->GetCommandList();

            _gpuProfiler->BeginEvent(commandList, _frameTimerID);

            commandList->Close();
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
            _endFrameTask = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
            _endFrameTask->SetName("End Frame Task");
            rhi::CommandList* commandList = _endFrameTask->GetCommandList();

            _gpuProfiler->EndEvent(commandList, _frameTimerID);
            _gpuProfiler->ResolveTimestamps(commandList);

            commandList->Close();
        }
#endif

        for (std::uint32_t passIndex : _sortedPasses)
        {
            ITask* currentPreTask = _GPUTasks[passIndex * TASKS_PER_PASS];
            ITask* currentTask = _GPUTasks[passIndex * TASKS_PER_PASS + 1];
            ITask* currentPostTask = _GPUTasks[passIndex * TASKS_PER_PASS + 2];

            for (std::uint32_t adjacentPassIndex : _adjacencyLists[passIndex])
            {
                ITask* dependentTask = _GPUTasks[adjacentPassIndex * TASKS_PER_PASS];
                dependentTask->AddDependency(currentTask->GetName());
            }

            currentTask->AddDependency(currentPreTask->GetName());
            currentPostTask->AddDependency(currentTask->GetName());
        }

#if ENABLE_PROFILING
        if (_gpuProfiler)
        {
            ITask* firstTask = _GPUTasks[_sortedPasses.front() * TASKS_PER_PASS];
            ITask* preLastTask = _GPUTasks[(_sortedPasses.back() - 1) * TASKS_PER_PASS + 2];
            ITask* lastTask = _GPUTasks[_sortedPasses.back() * TASKS_PER_PASS];

            firstTask->AddDependency(_beginFrameTask->GetName());
            _endFrameTask->AddDependency(lastTask->GetName());
        }
#endif
    }

    void RenderGraph::BuildAdjacencyLists()
    {
        std::uint32_t passesCount = static_cast<std::uint32_t>(_passes.size());

        _adjacencyLists.clear();
        _adjacencyLists.resize(passesCount);

        for (std::uint32_t passIndex = 0; passIndex < passesCount; ++passIndex)
        {
            std::shared_ptr<IRenderPass>& pass = _passes[passIndex];
            std::vector<std::uint32_t>& passAdjacencyList = _adjacencyLists[passIndex];

            for (std::uint32_t i = passIndex + 1; i < passesCount; ++i)
            {
                std::shared_ptr<IRenderPass>& otherPass = _passes[i];

                for (RGBufferId readId : otherPass->_bufferReads)
                {
                    const std::vector<rg::RGBufferId>& passWriteIds = pass->_bufferWrites;
                    if (std::find(passWriteIds.cbegin(), passWriteIds.cend(), readId) != passWriteIds.cend())
                    {
                        passAdjacencyList.push_back(i);
                        break;
                    }
                }
                for (RGTextureId readId : otherPass->_textureReads)
                {
                    const std::vector<rg::RGTextureId>& passWriteIds = pass->_textureWrites;
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

        std::function<void(std::uint32_t)> DFS = [&](std::uint32_t passIndex)
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
        std::vector<rhi::BufferBarrier> bufferBarriers;
        std::vector<rhi::TextureBarrier> textureBarriers;

        for (std::shared_ptr<IRenderPass> renderPass : _passes)
        {
            if (!renderPass->_bufferCreates.empty())
            {
                for (auto id : renderPass->_bufferCreates)
                {
                    std::shared_ptr<rhi::Buffer> buffer = _context.GetBuffer(id);
                    if (!buffer)
                    {
                        continue;
                    }

                    std::shared_ptr<IRenderPass> lastPass = nullptr;
                    std::shared_ptr<IRenderPass> currentPass = renderPass;

                    while (currentPass)
                    {
                        if (currentPass->_bufferStateMap.find(id) != currentPass->_bufferStateMap.end())
                        {
                            lastPass = currentPass;
                        }

                        currentPass = currentPass->_nextPass.lock();
                    }

                    if (lastPass && lastPass->_bufferStateMap.find(id) != lastPass->_bufferStateMap.end())
                    {
                        rhi::ResourceState lastState = lastPass->_bufferStateMap[id];
                        rhi::ResourceState currState = buffer->GetCurrentState();
                        if (lastState != currState)
                        {
                            bufferBarriers.push_back({ buffer, currState, lastState });
                        }
                    }
                }
            }

            if (!renderPass->_bufferReads.empty())
            {
                for (auto readId : renderPass->_bufferReads)
                {
                    if (std::find(renderPass->_bufferCreates.cbegin(), renderPass->_bufferCreates.cend(), readId) != renderPass->_bufferCreates.cend())
                    {
                        continue;
                    }

                    int refCount = 1;

                    for (std::shared_ptr<IRenderPass> p : _passes)
                    {
                        if (p != renderPass && p->_bufferStateMap.find(readId) != p->_bufferStateMap.end())
                        {
                            refCount++;
                            break;
                        }
                    }

                    if (refCount <= 1)
                    {
                        std::shared_ptr<rhi::Buffer> buffer = _context.GetBuffer(readId);

                        if (buffer)
                        {
                            rhi::ResourceState lastState = renderPass->_bufferStateMap[readId];
                            rhi::ResourceState currState = buffer->GetCurrentState();
                            if (lastState != currState)
                            {
                                bufferBarriers.push_back({ buffer, currState, lastState });
                            }
                        }
                    }
                }
            }
        }

        for (std::shared_ptr<IRenderPass> renderPass : _passes)
        {
            if (!renderPass->_textureCreates.empty())
            {
                for (auto id : renderPass->_textureCreates)
                {
                    std::shared_ptr<rhi::Texture> texture = _context.GetTexture(id);
                    if (!texture)
                    {
                        continue;
                    }

                    std::shared_ptr<IRenderPass> lastPass = nullptr;
                    std::shared_ptr<IRenderPass> currentPass = renderPass;

                    while (currentPass)
                    {
                        if (currentPass->_textureStateMap.find(id) != currentPass->_textureStateMap.end())
                        {
                            lastPass = currentPass;
                        }

                        currentPass = currentPass->_nextPass.lock();
                    }

                    if (lastPass && lastPass->_textureStateMap.find(id) != lastPass->_textureStateMap.end())
                    {
                        rhi::ResourceState lastState = lastPass->_textureStateMap[id];
                        rhi::ResourceState currState = texture->GetCurrentState();
                        if (lastState != currState)
                        {
                            textureBarriers.push_back({ texture, currState, lastState });
                        }
                    }
                }
            }

            if (!renderPass->_textureReads.empty())
            {
                for (auto readId : renderPass->_textureReads)
                {
                    if (std::find(renderPass->_textureCreates.cbegin(), renderPass->_textureCreates.cend(), readId) != renderPass->_textureCreates.cend())
                    {
                        continue;
                    }

                    int refCount = 1;

                    for (std::shared_ptr<IRenderPass> p : _passes)
                    {
                        if (p != renderPass && p->_textureStateMap.find(readId) != p->_textureStateMap.end())
                        {
                            refCount++;
                            break;
                        }
                    }

                    if (refCount <= 1)
                    {
                        std::shared_ptr<rhi::Texture> texture = _context.GetTexture(readId);

                        if (texture)
                        {
                            rhi::ResourceState lastState = renderPass->_textureStateMap[readId];
                            rhi::ResourceState currState = texture->GetCurrentState();
                            if (lastState != currState)
                            {
                                textureBarriers.push_back({ texture, currState, lastState });
                            }
                        }
                    }
                }
            }
        }

        ITask* task = _taskAllocator->AllocateTask(rhi::CommandListType::Graphics);
        task->SetName("Resource State Transition");

        rhi::CommandList* commandList = task->GetCommandList();

        commandList->TransitionBarriers(bufferBarriers);
        commandList->TransitionBarriers(textureBarriers);
        commandList->Close();
    }
} // namespace rg
