#pragma once

#include "RenderGraph/Interfaces.h"
#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPass.h"

namespace rg::mt
{
    struct PassWork
    {
        IRenderPass* RenderPass = nullptr;
        RenderContext* Context = nullptr;
        ITask* PreExecuteTask = nullptr;
        ITask* ExecuteTask = nullptr;
        ITask* PostExecuteTask = nullptr;
    };

    class PassWorker
    {
    public:
        using WorkFunc = std::function<void(RenderContext&, ITask*)>;
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

