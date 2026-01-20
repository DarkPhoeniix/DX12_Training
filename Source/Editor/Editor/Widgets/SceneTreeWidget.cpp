#include "EditorPCH.h"

#include "SceneTreeWidget.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components/Camera.h"

namespace gui
{
    SceneTreeWidget::SceneTreeWidget(std::shared_ptr<Editor> editor)
        : IWidget(editor)
    {
    }

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

        ImGui::BeginChild("Scene Tree", {0, ImGui::GetWindowHeight() * 0.4f}, ImGuiChildFlags_FrameStyle);
        ImGui::SeparatorText("Scene Hierarchy");
        for (const auto& root : _editor->GetScene()->GetRootNodes())
        {
            Update(root);
        }

        ImGui::EndChild();
    }

    void SceneTreeWidget::Update(std::shared_ptr<scene::Entity> entity)
    {
        bool hasChildren = entity->GetChildrenNodes().empty();
        bool isSelected = _editor->GetSelectedEntity() == entity;
        ImGuiTreeNodeFlags flags = 0;
        flags |= hasChildren ? ImGuiTreeNodeFlags_Leaf : 0;
        flags |= isSelected ? ImGuiTreeNodeFlags_Selected : 0;

        if (ImGui::TreeNodeEx(entity->GetName().c_str(), flags))
        {
            if (ImGui::IsItemClicked())
            {
                _editor->SetSelectedEntity(entity);
            }

            for (std::shared_ptr<scene::Entity> child : entity->GetChildrenNodes())
            {
                Update(child);
            }
            ImGui::TreePop();
        }
    }
} // namespace gui
