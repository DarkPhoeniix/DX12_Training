#pragma once

#include "WorkQueue.h"
#include "PassWorker.h"

namespace rg::mt
{
    class PassWorkerManager
    {
    public:
        PassWorkerManager(std::uint32_t workersCount);
        ~PassWorkerManager() = default;

        void Submit(PassWork work);
        void Wait();

    private:
        void OnWorkerFinished();
        bool AllWorkersFree();

        WorkQueue _workQueue;
        std::vector<PassWorker> _workers;

        std::condition_variable _condition;
        std::mutex _mutex;
    };
} // namespace rg::mt
