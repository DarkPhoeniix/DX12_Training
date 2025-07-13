#include "RenderGraphPCH.h"

#include "PassWorkerManager.h"

namespace rg::mt
{
    PassWorkerManager::PassWorkerManager(std::uint32_t workersCount)
        : _workers(workersCount)
    {
    }

    void PassWorkerManager::Submit(PassWork work)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        for (PassWorker& worker : _workers)
        {
            if (worker.IsFree())
            {
                worker.AssignWork(std::move(work), std::bind(&PassWorkerManager::OnWorkerFinished, this));
                return;
            }
        }

        _workQueue.Push(std::move(work));
    }

    void PassWorkerManager::Wait()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this]() { return AllWorkersFree() && _workQueue.IsEmpty(); });
    }

    void PassWorkerManager::OnWorkerFinished()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_workQueue.IsEmpty())
        {
            for (PassWorker& worker : _workers)
            {
                if (worker.IsFree())
                {
                    worker.AssignWork(std::move(_workQueue.Pop()), std::bind(&PassWorkerManager::OnWorkerFinished, this));
                    return;
                }
            }
        }

        _condition.notify_one();
    }

    bool PassWorkerManager::AllWorkersFree()
    {
        for (PassWorker& worker : _workers)
        {
            if (!worker.IsFree())
            {
                return false;
            }
        }

        return true;
    }
} // namespace rg::mt
