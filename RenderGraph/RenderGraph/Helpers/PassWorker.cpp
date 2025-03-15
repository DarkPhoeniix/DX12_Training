#include "RenderGraphPCH.h"

#include "PassWorker.h"

namespace rg::mt
{
    PassWorker::PassWorker()
        : _isFree(true)
        , _exit(false)
    {
        _thread = std::thread(&PassWorker::Execute, this);
    }

    PassWorker::~PassWorker()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _exit = true;
        }
        _condition.notify_one();

        if (_thread.joinable())
        {
            _thread.join();
        }

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
            std::function<void()> onComplete;
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [this]() { return !_isFree || _exit; });

                if (_exit)
                {
                    return;
                }

                if (!_work.RenderPass)  // Ensure work is assigned
                {
                    continue;
                }

                onComplete = _callback;
            }

            _work.RenderPass->Execute(*_work.Context, *_work.Task);

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
