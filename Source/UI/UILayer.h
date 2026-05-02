#pragma once

#include "Renderer/Window/WindowEvent.h"

#include <memory>
#include <string>
#include <cstdint>

namespace rhi
{
    class Device;
    class CommandList;
} // namespace rhi

namespace ui
{
    class UILayer
    {
    public:
        virtual ~UILayer() = default;

        virtual void Begin() = 0;
        virtual void End(rhi::CommandList* commandList) = 0;

        virtual void OnWindowEvent(const core::WindowEvent& event) = 0;
    };

    std::unique_ptr<UILayer> CreateUILayer(rhi::Device* device, void* windowHandle);
} // namespace ui
