#pragma once

#include "IRenderPass.h"

#include "GUI/Widgets/SceneTreeWidget.h"

namespace render
{
    class GUIPass : public IRenderPass
    {
    public:
        // Inherited via IRenderPass
        void Inititalize() override;
        void Destroy() override;

        void Execute() override;

    private:
        std::shared_ptr<gui::SceneTreeWidget> _sceneWidget;
    };
} // namespace render
