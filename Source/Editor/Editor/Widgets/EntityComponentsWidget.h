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
    class EntityComponentsWidget : public IWidget
    {
    public:
        EntityComponentsWidget(std::shared_ptr<Editor> editor);
        ~EntityComponentsWidget() = default;

        void Init() override;
        void Destroy() override;

        void Update() override;

    private:
        void CreateComponentWidget(std::shared_ptr<scene::Animation> animation);
        void CreateComponentWidget(std::shared_ptr<scene::Armature> armature);
        void CreateComponentWidget(std::shared_ptr<scene::Camera> camera);
        void CreateComponentWidget(std::shared_ptr<scene::Light> light);
        void CreateComponentWidget(std::shared_ptr<scene::Material> material);
        void CreateComponentWidget(std::shared_ptr<scene::Mesh> mesh);
        void CreateComponentWidget(std::shared_ptr<scene::Skybox> skybox);
        void CreateComponentWidget(std::shared_ptr<scene::Transformation> transformation);
    };
} // namespace gui
