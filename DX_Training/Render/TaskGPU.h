#pragma once

class TaskGPU
{
public:
    void SetCommandQueue(ComPtr<ID3D12CommandQueue> commandQueue);

    void AddCommandList(ComPtr<ID3D12GraphicsCommandList2>);
    std::vector<ComPtr<ID3D12GraphicsCommandList2>> GetCommandLists() const;

    void AddDependency(const std::string& taskName);
    std::vector<std::string> GetDependencies() const;

    void SetName(const std::string& name);
    const std::string& GetName() const;

private:
    std::vector<ComPtr<ID3D12GraphicsCommandList2>> _commandLists;
    ComPtr<ID3D12CommandQueue> _commandQueue;

    Sync* sync = nullptr;
    std::vector<std::string> _dependencies;

    std::string _name;
};

