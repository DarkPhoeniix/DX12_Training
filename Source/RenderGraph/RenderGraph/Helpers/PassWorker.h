#pragma once

#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPass.h"

#include "Renderer/Render/Frame/TaskGPU.h"

namespace rg::mt
{
    struct PassWork
    {
        IRenderPass* RenderPass = nullptr;
        RenderContext* Context = nullptr;
        TaskGPU* PreExecuteTask = nullptr;
        TaskGPU* ExecuteTask = nullptr;
        TaskGPU* PostExecuteTask = nullptr;
    };

    class PassWorker
    {
    public:
        using WorkFunc = std::function<void(RenderContext&, TaskGPU&)>;
        using CallbackFunc = std::function<void()>;

        PassWorker();
        ~PassWorker();

        void AssignWork(PassWork&& work, CallbackFunc&& callback);
        void Wait();

        bool IsFree();

    private:
        void Execute();

        std::thread _thread;
        std::atomic<bool> _isFree;
        std::atomic<bool> _exit;

        std::mutex _mutex;
        std::condition_variable _condition;

        PassWork _work;
        CallbackFunc _callback;
    };
} // namespace rg::mt

