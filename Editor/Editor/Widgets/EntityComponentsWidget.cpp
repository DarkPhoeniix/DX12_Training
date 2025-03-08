#include "EditorPCH.h"

#include "EntityComponentsWidget.h"

#include "Scene/Entity/Entity.h"
#include "Scene/Entity/Components/Animation.h"
#include "Scene/Entity/Components/Armature.h"
#include "Scene/Entity/Components/Camera.h"
#include "Scene/Entity/Components/Light.h"
#include "Scene/Entity/Components/Material.h"
#include "Scene/Entity/Components/Mesh.h"
#include "Scene/Entity/Components/Skybox.h"
#include "Scene/Entity/Components/Transformation.h"

namespace gui
{
    namespace
    {
        float ToDegrees(float radians)
        {
            return radians * (180.0f / DirectX::XM_PI);
        }

        float ToRadians(float degrees)
        {
            return degrees * (DirectX::XM_PI / 180.0f);
        }

        DirectX::XMFLOAT3 GetEulerAnglesFromQuaternionManual(DirectX::XMVECTOR quaternion)
        {
            DirectX::XMFLOAT4 q;
            DirectX::XMStoreFloat4(&q, quaternion);

            DirectX::XMFLOAT3 euler, result;

            float xx = q.x * q.x;
            float yy = q.y * q.y;
            float zz = q.z * q.z;

            float m31 = 2.0f * q.x * q.z + 2.0f * q.y * q.w;
            float m32 = 2.0f * q.y * q.z - 2.0f * q.x * q.w;
            float m33 = 1.0f - 2.0f * xx - 2.0f * yy;

            float cy = sqrtf(m33 * m33 + m31 * m31);
            float cx = atan2f(-m32, cy);
            if (cy > 16.0f * FLT_EPSILON)
            {
                float m12 = 2.0f * q.x * q.y + 2.0f * q.z * q.w;
                float m22 = 1.0f - 2.0f * xx - 2.0f * zz;

                result = { cx, atan2f(m31, m33), atan2f(m12, m22) };
            }
            else // handle gimbal lock
            {
                float m11 = 1.0f - 2.0f * yy - 2.0f * zz;
                float m21 = 2.0f * q.x * q.y - 2.0f * q.z * q.w;

                result = { cx, 0.0f, atan2f(-m21, m11) };
            }

            result.x = ToDegrees(result.x);
            result.y = ToDegrees(result.y);
            result.z = ToDegrees(result.z);

            return result;
        }
    } // namespace unnamed

    EntityComponentsWidget::EntityComponentsWidget(std::shared_ptr<Editor> editor)
        : IWidget(editor)
    {
    }

    void EntityComponentsWidget::Init()
    {
        IWidget::Init();
    }

    void EntityComponentsWidget::Destroy()
    {
        IWidget::Destroy();
    }

    void EntityComponentsWidget::Update()
    {
        IWidget::Update();

        ImGui::BeginChild("Entity Components", { 0.0f, 0.0f }, ImGuiChildFlags_FrameStyle);

        std::shared_ptr<scene::Entity> entity = _editor->GetSelectedEntity();
        if (!entity)
        {
            ImGui::SeparatorText("Entity Components");
            ImGui::EndChild();
            return;
        }

        ImGui::SeparatorText((entity->GetName() + " Components").c_str());

        for (const auto& component : entity->GetComponents())
        {
            if (component->ComponentName == "Animation")
            {
                CreateComponentWidget(static_cast<scene::Animation*>(component.get()));
            }
            else if (component->ComponentName == "Armature")
            {
                CreateComponentWidget(static_cast<scene::Armature*>(component.get()));
            }
            else if (component->ComponentName == "Camera")
            {
                CreateComponentWidget(static_cast<scene::Camera*>(component.get()));
            }
            else if (component->ComponentName == "Light")
            {
                CreateComponentWidget(static_cast<scene::Light*>(component.get()));
            }
            else if (component->ComponentName == "Material")
            {
                CreateComponentWidget(static_cast<scene::Material*>(component.get()));
            }
            else if (component->ComponentName == "Mesh")
            {
                CreateComponentWidget(static_cast<scene::Mesh*>(component.get()));
            }
            else if (component->ComponentName == "Skybox")
            {
                CreateComponentWidget(static_cast<scene::Skybox*>(component.get()));
            }
            else if (component->ComponentName == "Transformation")
            {
                CreateComponentWidget(static_cast<scene::Transformation*>(component.get()));
            }
        }

        ImGui::EndChild();
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Animation* animation)
    {
        if (ImGui::CollapsingHeader(animation->ComponentName.c_str()))
        {
            ImGui::Text("Frames: %i", animation->Frames.size());
            ImGui::DragFloat("Duration", &animation->Duration);
            ImGui::DragFloat("Ticks per sec", &animation->TicksPerSecond, 0.1f, 1.0f, 100.0f);
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Armature* armature)
    {
        if (ImGui::CollapsingHeader(armature->ComponentName.c_str()))
        {
            ImGui::Text("Bones: %i", armature->GetBones().size());
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Camera* camera)
    {
        if (ImGui::CollapsingHeader(camera->ComponentName.c_str()))
        {
            if (ImGui::DragFloat("Field of view", &camera->FoV, 0.1f, 5.0f, 150.0f))
            {
                camera->Update();
            }
            if (ImGui::DragFloat("Near Z", &camera->NearZ, 0.1f, 0.01f, 100.0f))
            {
                camera->Update();
            }
            if (ImGui::DragFloat("Far Z", &camera->FarZ, 0.1f, 0.1f, 10000.0f))
            {
                camera->Update();
            }
            if (ImGui::DragFloat("Speed", &camera->Speed, 1.0f, 1.0f, 1000.0f))
            {
                camera->Update();
            }
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Light* light)
    {
        if (ImGui::CollapsingHeader(light->ComponentName.c_str()))
        {
            DirectX::XMFLOAT3 direction, color, position;
            DirectX::XMStoreFloat3(&direction, light->Direction);
            DirectX::XMStoreFloat3(&color, light->Color);

            bool modified = false;

            if (ImGui::ColorPicker3("Color", &color.x, ImGuiColorEditFlags_DisplayRGB))
            {
                light->Color = DirectX::XMLoadFloat3(&color);
                modified = true;
            }
            ImGui::DragFloat("Intensity", &light->Intensity, 0.05f, 0.0f);
            ImGui::Checkbox("Cast shadows", &light->CastShadows);

            switch (light->Type)
            {
            case scene::LightType::Directional:
                ImGui::DragFloat3("Direction", &direction.x, 0.05f);
                break;

            case scene::LightType::Point:
                ImGui::DragFloat("Range", &light->Range, 0.1f, 0.0f);
                break;

            case scene::LightType::Spot:
                if (ImGui::DragFloat3("Direction", &direction.x, 0.05f, -1.0f, 1.0f))
                {
                    light->Direction = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction));
                    modified = true;
                }
                ImGui::DragFloat("Range", &light->Range, 0.1f, 0.0f);
                ImGui::DragFloat("Outer angle", &light->OuterAngle);
                ImGui::DragFloat("Inner angle", &light->InnerAngle);
            }
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Material* material)
    {
        if (ImGui::CollapsingHeader(material->ComponentName.c_str()))
        {
            ImGui::Text("Albedo: %s", material->Albedo->GetName().c_str());
            ImGui::Text("Metalness: %s", material->Metalness->GetName().c_str());
            ImGui::Text("Roughness: %s", material->Roughness->GetName().c_str());
            ImGui::Text("Normal map: %s", material->NormalMap->GetName().c_str());
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Mesh* mesh)
    {
        if (ImGui::CollapsingHeader(mesh->ComponentName.c_str()))
        {
            ImGui::Text("Triangles: %i", (mesh->IndexData.size() / 3));
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Skybox* skybox)
    {
        if (ImGui::CollapsingHeader(skybox->ComponentName.c_str()))
        {
            ImGui::Text("Texture: %s", skybox->SkydomeTexture->GetName().c_str());
        }
    }

    void EntityComponentsWidget::CreateComponentWidget(scene::Transformation* transformation)
    {
        if (ImGui::CollapsingHeader(transformation->ComponentName.c_str()))
        {
            DirectX::XMVECTOR pos, rot, sc;
            DirectX::XMMatrixDecompose(&sc, &rot, &pos, transformation->Transform);

            DirectX::XMFLOAT3 location, euler, scale;
            DirectX::XMStoreFloat3(&location, pos);
            euler = GetEulerAnglesFromQuaternionManual(rot);
            DirectX::XMFLOAT3 prev = euler;

            DirectX::XMStoreFloat3(&scale, sc);

            bool modified = false;

            if (ImGui::DragFloat3("Location", &location.x, 0.05f))
            {
                pos = DirectX::XMLoadFloat3(&location);
                modified = true;
            }

            if (ImGui::DragFloat3("Rotation", &euler.x, 0.5f))
            {
                euler.x = Math::Clamp(euler.x, -90.0f, 90.0f);
                rot = DirectX::XMQuaternionRotationRollPitchYaw(ToRadians(euler.x), ToRadians(euler.y), ToRadians(euler.z));
                modified = true;
            }

            if (ImGui::DragFloat3("Scale", &scale.x, 0.001f, 0.001f))
            {
                sc = DirectX::XMLoadFloat3(&scale);
                modified = true;
            }

            if (modified)
            {
                transformation->Transform = DirectX::XMMatrixScalingFromVector(sc) *
                                            DirectX::XMMatrixRotationQuaternion(rot) *
                                            DirectX::XMMatrixTranslationFromVector(pos);
            }
        }
    }
} // namespace gui
