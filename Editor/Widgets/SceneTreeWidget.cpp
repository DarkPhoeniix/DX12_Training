#include "EditorPCH.h"

#include "SceneTreeWidget.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace gui
{
    SceneTreeWidget::SceneTreeWidget(const scene::Scene& scene)
        : _scene(&scene)
    {
    }

    void SceneTreeWidget::Init()
    {
        std::shared_ptr<scene::Entity> activeCamera = _scene->FindNodeByComponentName("Camera");
        scene::Camera* cameraComponent = activeCamera->GetComponentAs<scene::Camera>("Camera");
        _viewport = &cameraComponent->GetViewport();
    }

    void SceneTreeWidget::Destroy()
    {
    }

    void SceneTreeWidget::Update()
    {
        DirectX::XMUINT2 viewportSize = _viewport->GetSize();

        float positionX = (float)(viewportSize.x - (viewportSize.x * 0.2f));
        float positionY = 0.0f;
        float sizeX = (float)(viewportSize.x * 0.2f);
        float sizeY = (float)(viewportSize.y);

        ImGui::SetNextWindowPos({ positionX, positionY });
        ImGui::SetNextWindowSize({ sizeX, sizeY });

        ImGui::Begin("Scene Tree");
        for (const auto& root : _scene->GetRootNodes())
        {
            Update(root);
        }

        ImGui::End();
    }

    void SceneTreeWidget::Update(const std::shared_ptr<scene::Entity>& entity)
    {
        if (ImGui::TreeNodeEx(entity->GetName().c_str()))
        {
            for (const auto& child : entity->GetChildrenNodes())
            {
                Update(child);
            }

            ImGui::TreePop();
        }
    }
} // namespace gui
