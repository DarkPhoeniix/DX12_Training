#pragma once

#include "IWidget.h"

namespace scene
{
    class Entity;
    class Scene;
} // namespace scene

namespace gui
{
    class SceneTreeWidget : IWidget
    {
    public:
        void Init() override;
        void Destroy() override;

        void Update() override;

    private:
        void Update(const std::shared_ptr<scene::Entity>& entity);
    };
} // namespace gui
