#include "RenderGraphPCH.h"

#include "RenderPass.h"

#include "RenderContext.h"

#include "ResourceBarrier.h"

namespace rg
{
    IRenderPass::IRenderPass(const std::string& name, RenderPassType type)
        : _creates{}
        , _writes{}
        , _reads{}
        , _resourceStateMap{}
        , _refCount(0)
        , _type(type)
        , _name(name)
        , _gpuTimerID(Profiler::InvalidTimerID)
    {
    }

    void IRenderPass::PreExecute(RenderContext& context, TaskGPU& task)
    {
        std::vector<dx12::ResourceBarrier> barriers;

        for (auto& [id, state] : _resourceStateMap)
        {
            std::shared_ptr<dx12::Resource> resource = context.GetResource(id);

            bool flag = false;
            {
                std::shared_ptr<IRenderPass> firstPass = nullptr;
                std::shared_ptr<IRenderPass> currentPass = _prevPass.lock();
                while (currentPass)
                {
                    if (currentPass->_resourceStateMap.find(id) != currentPass->_resourceStateMap.end())
                    {
                        dx12::ResourceState prevState = currentPass->_resourceStateMap[id];
                        if (prevState != state)
                        {
                            barriers.push_back({ resource, prevState, state });
                        }
                        flag = true;
                        break;
                    }

                    currentPass = currentPass->_prevPass.lock();
                }
            }
            if (!flag)
            {
                std::shared_ptr<IRenderPass> lastPass = nullptr;
                std::shared_ptr<IRenderPass> currentPass = _nextPass.lock();
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
                    if (lastState != state)
                    {
                        barriers.push_back({ resource, lastState, state });
                        flag = true;
                    }
                }
            }
        }

        dx12::CommandList& commandList = *task.GetCommandLists().front();

        if (!barriers.empty())
        {
            commandList.TransitionBarriers(barriers);
        }

#if ENABLE_PROFILING
        context.GetGPUProfiler()->BeginEvent(commandList, _gpuTimerID);
#endif
        commandList.Close();
    }

    void IRenderPass::PostExecute(RenderContext& context, TaskGPU& task)
    {
        dx12::CommandList& commandList = *task.GetCommandLists().front();
#if ENABLE_PROFILING
        context.GetGPUProfiler()->EndEvent(commandList, _gpuTimerID);
#endif
        commandList.Close();
    }

    RenderPassType IRenderPass::GetType() const
    {
        return _type;
    }
} // namespace rg
