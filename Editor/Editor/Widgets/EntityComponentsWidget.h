#pragma once

#include "IWidget.h"
#include "Editor/Editor.h"

namespace scene
{
    class Animation;
    class Armature;
    class Camera;
    class Light;
    class Material;
    class Mesh;
    class Skybox;
    class Transformation;

    class Entity;
    class Scene;
} // namespace scene

namespace gui
{
    class EntityComponentsWidget : IWidget
    {
    public:
        EntityComponentsWidget(std::shared_ptr<Editor> editor);
        ~EntityComponentsWidget() = default;

        void Init() override;
        void Destroy() override;

        void Update() override;

    private:
        void CreateComponentWidget(scene::Animation* animation);
        void CreateComponentWidget(scene::Armature* armature);
        void CreateComponentWidget(scene::Camera* camera);
        void CreateComponentWidget(scene::Light* light);
        void CreateComponentWidget(scene::Material* material);
        void CreateComponentWidget(scene::Mesh* mesh);
        void CreateComponentWidget(scene::Skybox* skybox);
        void CreateComponentWidget(scene::Transformation* transformation);
    };
} // namespace gui
