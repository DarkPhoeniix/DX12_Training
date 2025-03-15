#include "RenderGraphPCH.h"

#include "WorkQueue.h"

namespace rg::mt
{
    void WorkQueue::Push(PassWork&& work)
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        _works.push(std::move(work));
    }

    PassWork&& WorkQueue::Pop()
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        PassWork& work = _works.front();
        _works.pop();

        return std::move(work);
    }

    PassWork& WorkQueue::Top()
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        return _works.front();
    }

    bool WorkQueue::IsEmpty() const
    {
        return _works.empty();
    }
}
