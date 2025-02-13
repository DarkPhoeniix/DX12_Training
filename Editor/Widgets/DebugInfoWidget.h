#pragma once

#include "IWidget.h"

namespace gui
{
    class DebugInfoWidget : public IWidget
    {
    public:
        void Init() override;
        void Destroy() override;

        void Update() override;
    };
} // namespace gui
