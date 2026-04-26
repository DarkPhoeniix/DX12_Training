#include "EditorPCH.h"

#include "IWidget.h"

#include "Scene/Entity/Components/Camera.h"

namespace gui
{
    IWidget::IWidget(Editor* editor)
        : _editor(editor)
    {
    }

    void IWidget::Init()
    {
    }

    void IWidget::Destroy()
    {
    }

    void IWidget::Update()
    {
    }
} // namespace gui
