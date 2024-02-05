#pragma once

#include "Allocators.h"
#include "Executor.h"
#include "TaskGPU.h"

class Frame
{
public:
    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12PipelineState> pipelineState)
    {
        Executor* exec = _allocs->Obtain(type);
        _currentTasks.push_back(exec);

        exec->Reset(pipelineState);
        exec->SetFree(false);

        _tasks.push_back({});
        TaskGPU* task = &_tasks.back();
        if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            task->SetCommandQueue(_queueStream);
        }
        else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        {
            task->SetCommandQueue(_queueCompute);
        }
        else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
        {
            task->SetCommandQueue(_queueCopy);
        }
        task->AddCommandList(exec->GetCommandList().Get());
        task->sync = _syncs->Obtain();
        task->sync->setFree(false);
        task->sync->value++;

        return task;
    }

    void WaitCPU()
    {
        if (_syncFrame)
        {
            _syncFrame->WaitForCPU();
            _syncFrame->IsFree = true;
            _syncFrame = nullptr;
        }
    }

    void ResetGPU()
    {
        for (auto& task : _executedTasks)
        {
            task->SetFree(true);
        }

        for (auto& task : _tasks)
        {
            task.sync->SetFree(true);
        }

        _tasks.clear();
        _executedTasks.clear();
        _executedTasks = std::move(_currentTasks);
    }

    TaskGPU* GetTask(const std::string& name)
    {
        for (auto& task : _tasks)
        {
            if (task.GetName() == name)
            {
                return &task;
            }
        }

        return nullptr;
    }

private:
    std::vector<Executor*> _currentTasks;
    std::vector<Executor*> _executedTasks;

    ComPtr<ID3D12CommandQueue> _queueStream;
    ComPtr<ID3D12CommandQueue> _queueCopy;
    ComPtr<ID3D12CommandQueue> _queueCompute;

    Sync* _syncFrame = nullptr;
    Allocators* _allocs;
    Syncs* _syncs;

    unsigned int _index = 0;
    Frame* _prev = nullptr;
    Frame* _next = nullptr;

    std::vector<TaskGPU> _tasks;
};
