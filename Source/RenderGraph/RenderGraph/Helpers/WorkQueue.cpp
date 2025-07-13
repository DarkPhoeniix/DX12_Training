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

        // ** If empty() is true, the behavior is undefined.
        ASSERT(!_works.empty(), "Trying to pop from an empty work queue.");
        PassWork& work = _works.front();
        _works.pop();

        return std::move(work);
    }

    PassWork& WorkQueue::Top()
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        ASSERT(!_works.empty(), "Trying to pop from an empty work queue.");

        return _works.front();
    }

    const PassWork& WorkQueue::Top() const
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        ASSERT(!_works.empty(), "Trying to pop from an empty work queue.");

        return _works.front();
    }

    bool WorkQueue::IsEmpty() const
    {
        return _works.empty();
    }
} // namespace rg::mt
