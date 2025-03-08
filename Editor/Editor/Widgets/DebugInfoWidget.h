#pragma once

#include "IWidget.h"
#include "Editor/Editor.h"

namespace gui
{
    class DebugInfoWidget : public IWidget
    {
    public:
        DebugInfoWidget(std::shared_ptr<Editor> editor);
        ~DebugInfoWidget() = default;

        void Init() override;
        void Destroy() override;

        void Update() override;
    };
} // namespace gui
