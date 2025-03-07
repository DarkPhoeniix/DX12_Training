#include "EditorPCH.h"

#include "SceneTreeWidget.h"

#include "Editor.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace gui
{
    void SceneTreeWidget::Init()
    {
        IWidget::Init();
    }

    void SceneTreeWidget::Destroy()
    {
        IWidget::Destroy();
    }

    void SceneTreeWidget::Update()
    {
        IWidget::Update();

        scene::Scene* scene = Editor::GetScene();
        std::shared_ptr<scene::Entity> activeCamera = scene->FindNodeByComponentName("Camera");
        scene::Camera* cameraComponent = activeCamera->GetComponentAs<scene::Camera>("Camera");
        _viewport = &cameraComponent->GetViewport();

        DirectX::XMUINT2 viewportSize = _viewport->GetSize();

        float positionX = (float)(viewportSize.x - (viewportSize.x * 0.2f));
        float positionY = 0.0f;
        float sizeX = (float)(viewportSize.x * 0.2f);
        float sizeY = (float)(viewportSize.y);

        ImGui::BeginChild("Scene Tree", {0, sizeY * 0.4f}, ImGuiChildFlags_FrameStyle);
        ImGui::SeparatorText("Scene Hierarchy");
        for (const auto& root : scene->GetRootNodes())
        {
            Update(root);
        }

        ImGui::EndChild();
    }

    void SceneTreeWidget::Update(const std::shared_ptr<scene::Entity>& entity)
    {
        bool hasChildren = entity->GetChildrenNodes().empty();
        bool isSelected = Editor::GetSelectedEntity() == entity;
        ImGuiTreeNodeFlags flags = 0;
        flags |= hasChildren ? ImGuiTreeNodeFlags_Leaf : 0;
        flags |= isSelected ? ImGuiTreeNodeFlags_Selected : 0;

        if (ImGui::TreeNodeEx(entity->GetName().c_str(), flags))
        {
            if (ImGui::IsItemClicked())
            {
                Editor::SetSelectedEntity(entity);
            }

            for (const auto& child : entity->GetChildrenNodes())
            {
                Update(child);
            }
            ImGui::TreePop();
        }
    }
} // namespace gui
