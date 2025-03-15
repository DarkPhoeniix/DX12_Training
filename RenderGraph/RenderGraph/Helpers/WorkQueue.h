#pragma once

#include "PassWorker.h"

#include "Render/Frame/TaskGPU.h"
#include "RenderGraph/RenderContext.h"
#include "RenderGraph/RenderPass.h"

#include <queue>
#include <mutex>

namespace rg::mt
{
    class WorkQueue
    {
    public:
        void Push(PassWork&& work);
        [[nodiscard]] PassWork&& Pop();

        [[nodiscard]] PassWork& Top();

        bool IsEmpty() const;

    private:
        std::queue<PassWork> _works;
        std::mutex _queueMutex;
    };
}
