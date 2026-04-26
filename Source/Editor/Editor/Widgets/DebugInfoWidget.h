#pragma once

#include "IWidget.h"
#include "Editor/Editor.h"

namespace gui
{
    class DebugInfoWidget : public IWidget
    {
    public:
        DebugInfoWidget(rhi::Device* device, Editor* editor);
        ~DebugInfoWidget() = default;

        void Init() override;
        void Destroy() override;

        void Update() override;

    private:
        bool _openDetailedCPUTime;

        rhi::Device* _device;
    };
} // namespace gui
