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
        IWidget(Editor* editor);
        virtual ~IWidget() = default;

        virtual void Init();
        virtual void Destroy();

        virtual void Update();

    protected:
        Editor* _editor;
    };
} // namespace gui
