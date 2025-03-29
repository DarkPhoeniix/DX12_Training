#pragma once

#include "PassWorker.h"

#include <queue>

namespace rg::mt
{
    class WorkQueue
    {
    public:
        void Push(PassWork&& work);
        [[nodiscard]] PassWork&& Pop();

        PassWork& Top();
        const PassWork& Top() const;

        bool IsEmpty() const;

    private:
        std::queue<PassWork> _works;
        mutable std::mutex _queueMutex;
    };
} // namespace rg::mt
