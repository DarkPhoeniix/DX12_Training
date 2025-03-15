#pragma once

#include "Render/Frame/TaskGPU.h"
#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPass.h"

#include <thread>
#include <mutex>

namespace rg::mt
{
    struct PassWork
    {
        IRenderPass* RenderPass = nullptr;
        RenderContext* Context = nullptr;
        TaskGPU* Task = nullptr;
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
        WorkFunc _func;
        CallbackFunc _callback;
    };
}

