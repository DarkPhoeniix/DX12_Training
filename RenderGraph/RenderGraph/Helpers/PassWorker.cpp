#include "RenderGraphPCH.h"

#include "PassWorker.h"

namespace rg::mt
{
    PassWorker::PassWorker()
        : _isFree(true)
    {
        _thread = std::thread(&PassWorker::Execute, this);
    }

    void PassWorker::AssignWork(PassWork&& work, CallbackFunc&& callback)
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _work = std::move(work);
            _callback = std::move(callback);
            _isFree = false;
        }
        _condition.notify_one();
    }

    void PassWorker::Execute()
    {
        while (true)
        {
            PassWork work;
            std::function<void()> onComplete;

            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this]() { return !_isFree; });

                if (!_work.RenderPass)  // Ensure work is assigned
                    continue;

                work = _work;
                onComplete = _callback;
            }

            work.RenderPass->Execute(*work.Context, *work.Task);

            OutputDebugStringA(" - - - > Worker is finished: ");
            OutputDebugStringA(work.Task->GetName().c_str());
            OutputDebugStringA("\n");

            {
                std::lock_guard<std::mutex> lock(_mutex);
                _work = {}; // Reset work
                _isFree = true;
            }

            onComplete();
        }
    }

    void PassWorker::Wait()
    {
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    bool PassWorker::IsFree()
    {
        return _isFree;
    }
}
