#pragma once

#include "Editor/Editor.h"

namespace scene
{
    class Viewport;
}

namespace gui
{
    class IWidget
    {
    public:
        IWidget(std::shared_ptr<Editor> editor);
        virtual ~IWidget() = default;

        virtual void Init();
        virtual void Destroy();

        virtual void Update();

    protected:
        std::shared_ptr<Editor> _editor;
    };
} // namespace gui
