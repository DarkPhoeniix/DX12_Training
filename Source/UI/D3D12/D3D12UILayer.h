#pragma once

#include "UILayer.h"

#include <memory>
#include <string>
#include <cstdint>

namespace rhi
{
    class DescriptorHeap;
} // namespace rhi

namespace ui
{
    class D3D12UILayer final : public UILayer
    {
    public:
        D3D12UILayer(rhi::Device* device, void* windowHandle);
        D3D12UILayer(const D3D12UILayer&) = delete;
        D3D12UILayer(D3D12UILayer&&) = default;
        ~D3D12UILayer() override;

        D3D12UILayer& operator=(const D3D12UILayer&) = delete;
        D3D12UILayer& operator=(D3D12UILayer&&) = default;

        void Begin() override;
        void End(rhi::CommandList* commandList) override;

        void OnWindowEvent(const core::WindowEvent& windowEvent) override;

    private:
        std::unique_ptr<rhi::DescriptorHeap> _descriptorHeap;

        rhi::Device* _device;
    };
} // namespace ui
