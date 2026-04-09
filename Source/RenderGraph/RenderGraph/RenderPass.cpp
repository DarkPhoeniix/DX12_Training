
#include "RenderGraphPCH.h"

#include "RenderPass.h"

#include "RenderContext.h"

#include "RHI/ResourceBarrier.h"

namespace rg
{
    IRenderPass::IRenderPass(const std::string& name, RenderPassType type)
        : _bufferCreates{}
        , _textureCreates{}
        , _bufferWrites{}
        , _textureWrites{}
        , _bufferReads{}
        , _textureReads{}
        , _bufferStateMap{}
        , _textureStateMap{}
        , _refCount(0)
        , _type(type)
        , _name(name)
        , _gpuTimerID(Profiler::InvalidTimerID)
    {
    }

    void IRenderPass::PreExecute(RenderContext& context, ITask* task)
    {
        std::vector<rhi::BufferBarrier> bufferBarriers;
        std::vector<rhi::TextureBarrier> textureBarriers;

        for (auto& [id, state] : _bufferStateMap)
        {
            std::shared_ptr<rhi::Buffer> resource = context.GetBuffer(id);

            bool flag = false;
            {
                std::shared_ptr<IRenderPass> firstPass = nullptr;
                std::shared_ptr<IRenderPass> currentPass = _prevPass.lock();
                while (currentPass)
                {
                    if (currentPass->_bufferStateMap.find(id) != currentPass->_bufferStateMap.end())
                    {
                        rhi::ResourceState prevState = currentPass->_bufferStateMap[id];
                        if (prevState != state)
                        {
                            bufferBarriers.push_back({ resource, prevState, state });
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
                    if (currentPass->_bufferStateMap.find(id) != currentPass->_bufferStateMap.end())
                    {
                        lastPass = currentPass;
                    }

                    currentPass = currentPass->_nextPass.lock();
                }

                if (lastPass && lastPass->_bufferStateMap.find(id) != lastPass->_bufferStateMap.end())
                {
                    rhi::ResourceState lastState = lastPass->_bufferStateMap[id];
                    if (lastState != state)
                    {
                        bufferBarriers.push_back({ resource, lastState, state });
                        flag = true;
                    }
                }
            }
        }

        for (auto& [id, state] : _textureStateMap)
        {
            std::shared_ptr<rhi::Texture> texture = context.GetTexture(id);

            bool flag = false;
            {
                std::shared_ptr<IRenderPass> firstPass = nullptr;
                std::shared_ptr<IRenderPass> currentPass = _prevPass.lock();
                while (currentPass)
                {
                    if (currentPass->_textureStateMap.find(id) != currentPass->_textureStateMap.end())
                    {
                        rhi::ResourceState prevState = currentPass->_textureStateMap[id];
                        if (prevState != state)
                        {
                            textureBarriers.push_back({ texture, prevState, state });
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
                    if (currentPass->_textureStateMap.find(id) != currentPass->_textureStateMap.end())
                    {
                        lastPass = currentPass;
                    }

                    currentPass = currentPass->_nextPass.lock();
                }

                if (lastPass && lastPass->_textureStateMap.find(id) != lastPass->_textureStateMap.end())
                {
                    rhi::ResourceState lastState = lastPass->_textureStateMap[id];
                    if (lastState != state)
                    {
                        textureBarriers.push_back({ texture, lastState, state });
                        flag = true;
                    }
                }
            }
        }

        rhi::CommandList* commandList = task->GetCommandList();

        commandList->TransitionBarriers(bufferBarriers);
        commandList->TransitionBarriers(textureBarriers);

#if ENABLE_PROFILING
        context.GetGPUProfiler()->BeginEvent(commandList, _gpuTimerID);
#endif
        commandList->Close();
    }

    void IRenderPass::PostExecute(RenderContext& context, ITask* task)
    {
        rhi::CommandList* commandList = task->GetCommandList();
#if ENABLE_PROFILING
        context.GetGPUProfiler()->EndEvent(commandList, _gpuTimerID);
#endif
        commandList->Close();
    }

    RenderPassType IRenderPass::GetType() const
    {
        return _type;
    }
} // namespace rg
