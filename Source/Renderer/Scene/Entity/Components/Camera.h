#pragma once

#include "Renderer/Scene/Entity/Components/IComponent.h"
#include "Renderer/Scene/Volumes/FrustumVolume.h"

namespace scene
{
    class Camera : public IComponent
    {
    public:
        Camera();
        Camera(std::uint32_t width, std::uint32_t height);
        ~Camera() = default;

        void Update();
        void Update(DirectX::XMVECTOR direction);
        void Update(int pitch, int yaw);

        const DirectX::XMMATRIX& View() const;
        const DirectX::XMMATRIX& Projection() const;
        const DirectX::XMMATRIX& ViewProjection() const;

        [[nodiscard]] DirectX::XMVECTOR Right() const;
        [[nodiscard]] DirectX::XMVECTOR Up() const;
        [[nodiscard]] DirectX::XMVECTOR Look() const;

        const DirectX::XMVECTOR& Position() const;
        DirectX::XMVECTOR& Position();
        const DirectX::XMVECTOR& Target() const;
        DirectX::XMVECTOR& Target();

        void LookAt(const DirectX::XMVECTOR& pos, const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& up);

        const FrustumVolume& GetViewFrustum() const;

        void SetLens(float fov, float nearZ, float farZ);
        void SetFoV(float fov);
        float GetFoV() const;
        void SetNearZ(float nearZ);
        float GetNearZ() const;
        void SetFarZ(float farZ);
        float GetFarZ() const;

        void SetSpeed(float speed);
        float GetSpeed() const;

        void SetViewport(const rhi::Viewport& viewport);
        rhi::Viewport GetViewport() const;

        void SetSize(std::uint32_t width, std::uint32_t height);
        DirectX::XMUINT2 GetSize() const;

        float GetAspectRatio() const;

        void SetDepth(const DirectX::XMFLOAT2& depth);
        DirectX::XMFLOAT2 GetDepth() const;

        void SetScissorRectangle(const rhi::ScissorRect& rect);
        rhi::ScissorRect& GetScissorRectangle();
        const rhi::ScissorRect& GetScissorRectangle() const;

    private:
        // Constructs the view matrix based on the camera's basis
        // vectors and origin, relative to the world space
        void _BuildView();
        void _BuildProjection();
        void _UpdateFrustum();

        // Save camera related matrices.
        DirectX::XMMATRIX _view;
        DirectX::XMMATRIX _projection;
        DirectX::XMMATRIX _viewProjection;

        // Relative to world space.
        DirectX::XMVECTOR _position;
        DirectX::XMVECTOR _target;
        DirectX::XMVECTOR _up;

        // Frustum
        FrustumVolume _frustum;

        // Viewport
        rhi::Viewport _viewport;
        rhi::ScissorRect _scissorRectangle;
        float _aspectRatio;

        // Lens params
        float FoV;
        float NearZ;
        float FarZ;

        // Camera speed
        float Speed;
    };
} // namespace scene
