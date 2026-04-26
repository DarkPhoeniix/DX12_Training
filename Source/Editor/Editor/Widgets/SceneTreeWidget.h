#pragma once

#include "IWidget.h"
#include "Editor/Editor.h"

namespace scene
{
    class Entity;
    class Scene;
} // namespace scene

namespace gui
{
    class SceneTreeWidget : public IWidget
    {
    public:
        SceneTreeWidget(Editor* editor);
        ~SceneTreeWidget() = default;

        void Init() override;
        void Destroy() override;

        void Update() override;

    private:
        void Update(std::shared_ptr<scene::Entity> entity);
    };
} // namespace gui
