#pragma once

#include "Interfaces/IGame.h"
#include "RenderWindow.h"

#include "PoissonDiskDistribution.h"
#include "Objects/Camera.h"
#include "Utility/PipelineSettings.h"
#include "Model.h"

#include <vector>

struct Executor
{
    bool isFree = true;
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList2> commandList;

    void Allocate(ComPtr<ID3D12Device> device, D3D12_COMMAND_LIST_TYPE type)
    {
        device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
        device->CreateCommandList(0, type, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
        commandList->Close();
    }

    void Reset(ComPtr<ID3D12PipelineState> pipeline = nullptr)
    {
        //if (isFree)
        //{
        //    return;
        //}

        ID3D12PipelineState* pState = nullptr;
        if (pipeline)
        {
            pState = pipeline.Get();
        }

        allocator->Reset();
        commandList->Reset(allocator.Get(), pState);
    }
};

struct Sync
{
    bool isFree = true;
    ComPtr<ID3D12Fence> fence;
    HANDLE Event;
    UINT64 value = 0;

    

    void Init(ComPtr<ID3D12Device> device)
    {
        value = 0;

        device->CreateFence(value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        Event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    void WaitCPU()
    {
        if (fence->GetCompletedValue() >= value)
        {
            return;
        }

        this->fence->SetEventOnCompletion(value, Event);
        ::WaitForSingleObject(Event, DWORD_MAX);
    }
};

struct Allocators
{
    std::vector<Executor> streams;
    std::vector<Executor> computes;
    std::vector<Executor> copies;

    void Init(ComPtr<ID3D12Device> device)
    {
        Make(device, streams, D3D12_COMMAND_LIST_TYPE_DIRECT);
        Make(device, computes, D3D12_COMMAND_LIST_TYPE_COMPUTE);
        Make(device, copies, D3D12_COMMAND_LIST_TYPE_COPY);
    }

    Executor* Obtain(D3D12_COMMAND_LIST_TYPE type)
    {
        std::vector<Executor>* res;
        if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            res = &streams;
        }
        else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        {
            res = &computes;
        }
        else/* (type == D3D12_COMMAND_LIST_TYPE_COPY)*/
        {
            res = &copies;
        }

        for (auto& exec : *res)
        {
            if (exec.isFree)
            {
                return &exec;
            }
        }

        return nullptr;
    }

protected:
    void Make(ComPtr<ID3D12Device> device, std::vector<Executor>& vecExec, D3D12_COMMAND_LIST_TYPE type)
    {
        vecExec.resize(32);

        for (auto& exec : vecExec)
        {
            exec.Allocate(device, type);
            exec.isFree = true;
        }
    }
};

struct Syncs
{
    std::vector<Sync> syncss;

    void Init(ComPtr<ID3D12Device> device)
    {
        syncss.resize(32 + 32 + 32); // Direct + Compute + Copy

        for (auto& s : syncss)
        {
            s.Init(device);
            s.isFree = true;
        }
    }

    Sync* Obtain()
    {
        for (auto& s : syncss)
        {
            if (s.isFree)
            {
                return &s;
            }
        }

        return nullptr;
    }
};

struct TaskGPU
{
    std::vector<ComPtr<ID3D12GraphicsCommandList2>> cmd;
    ComPtr<ID3D12CommandQueue> queue;
    Sync* sync = nullptr;

    std::string name;

    //vec< string> depend;
    std::vector<std::string> dependency;
};

struct Frame
{
    std::vector<Executor*> currentTasks;
    std::vector<Executor*> executedTasks;

    ComPtr<ID3D12CommandQueue> queueStream;
    ComPtr<ID3D12CommandQueue> queueCopy;
    ComPtr<ID3D12CommandQueue> queueCompute;

    Sync* syncFrame = nullptr;
    Allocators* allocs;
    Syncs* syncs;

    unsigned int index = 0;
    Frame* prev = nullptr;
    Frame* next = nullptr;

    std::vector<TaskGPU> tasks;

    TaskGPU* CreateTask(D3D12_COMMAND_LIST_TYPE type, ComPtr<ID3D12PipelineState> pipelineState)
    {
        Executor* exec = allocs->Obtain(type);
        currentTasks.push_back(exec);

        exec->Reset(pipelineState);
        exec->isFree = false;

        tasks.push_back({});
        TaskGPU* task = &tasks.back();
        if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            task->queue = queueStream;
        }
        else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
        {
            task->queue = queueCompute;
        }
        else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
        {
            task->queue = queueCopy;
        }
        task->cmd.push_back(exec->commandList.Get());
        task->sync = syncs->Obtain();
        task->sync->isFree = false;
        task->sync->value++;

        return task;
    }

    void WaitCPU()
    {
        if (syncFrame)
        {
            syncFrame->WaitCPU();
            syncFrame->isFree = true;
            syncFrame = nullptr;
        }
    }

    void ResetGPU()
    {
        for (auto& task : executedTasks)
        {
            task->isFree = true;
        }

        for (auto& task : tasks)
        {
            task.sync->isFree = true;
        }

        tasks.clear();
        executedTasks.clear();
        executedTasks = std::move(currentTasks);
    }

    TaskGPU* GetTask(const std::string& name)
    {
        for (auto& task : tasks)
        {
            if (task.name == name)
            {
                return &task;
            }
        }

        return nullptr;
    }
};

class RenderCubeExample : public IGame
{
public:
    using super = IGame;

    RenderCubeExample(const std::wstring& name, int width, int height, bool vSync = false);
    //~RenderCubeExample();



    virtual bool loadContent() override;
    virtual void unloadContent() override;

protected:
    void onUpdate(UpdateEvent& e) override;
    void onRender(RenderEvent& e) override;
    void onKeyPressed(KeyEvent& e) override;
    void onMouseScroll(MouseScrollEvent& e) override;
    void onMouseMoved(MouseMoveEvent& e) override;
    void onMouseButtonPressed(MouseButtonEvent& e) override;
    void onMouseButtonReleased(MouseButtonEvent& e) override;
    void onResize(ResizeEvent& e) override;

private:
    void transitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

    void clearRTV(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

    void clearDepth(ComPtr<ID3D12GraphicsCommandList2> commandList,
        D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

    void updateBufferResource(ComPtr<ID3D12GraphicsCommandList2> commandList,
        ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
        size_t numElements, size_t elementSize, const void* bufferData,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    void resizeDepthBuffer(int width, int height);

    uint64_t _fenceValues[Window::BUFFER_COUNT] = {};

    ComPtr<ID3D12Resource> _vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;

    ComPtr<ID3D12Resource> _indexBuffer;
    D3D12_INDEX_BUFFER_VIEW _indexBufferView;

    ComPtr<ID3D12Resource> _depthBuffer;
    ComPtr<ID3D12DescriptorHeap> _DSVHeap;

    PipelineSettings _pipeline;

    ComPtr<ID3D12RootSignature> _rootComputeSignature;
    ComPtr<ID3D12PipelineState> _pipelineComputeState;

    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissorRect;

    bool _contentLoaded;

    float _spawnRate = 50.0f;
    float _deltaTimeLastSpawn = 0.0f;

    Model _model;

    PoissonDiskDistribution distribution;
    Resource* _ambient;
    Resource* _cubeTransformsRes[3];
    DirectX::XMMATRIX* _transfP[3];

    Resource* _dynamicData;
    Resource* _UAVRes;
    Resource* _readBack;
    ComPtr<ID3D12Heap> _pHeap;
    ComPtr<ID3D12DescriptorHeap> _descHeap;

    bool _isCameraMoving = false;
    Camera _camera;

    Frame frames[3];
    Frame* current;
    Allocators allocs;
    Syncs syncs;
};