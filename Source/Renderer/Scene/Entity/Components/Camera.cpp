#include "RendererPCH.h"

#include "Camera.h"

using namespace DirectX;

namespace
{
    constexpr float CAMERA_ROTATION_SPEED = 0.1f;
} // namespace unnamed

namespace scene
{
    Camera::Camera()
        : IComponent("Camera")
        , _view(XMMatrixIdentity())
        , _projection(XMMatrixIdentity())
        , _viewProjection(XMMatrixIdentity())
        , _position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
        , _up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
        , Speed(100.0f)
        , NearZ(0.1f)
        , FarZ(1.0f)
        , FoV(1.0f)
        , _viewport(1, 1)
        , _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
        , _aspectRatio(1.0f)
    {
        Update();
    }

    Camera::Camera(std::uint32_t width, std::uint32_t height)
        : IComponent("Camera")
        , _view(XMMatrixIdentity())
        , _projection(XMMatrixIdentity())
        , _viewProjection(XMMatrixIdentity())
        , _position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
        , _up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
        , Speed(100.0f)
        , _viewport(width, height)
        , _scissorRectangle(0, 0, LONG_MAX, LONG_MAX)
        , _aspectRatio(width / (float)height)
    {
        Update();
    }

    void Camera::Update()
    {
        _BuildView();
        _BuildProjection();
    }

    void Camera::Update(XMVECTOR direction)
    {
        XMMATRIX movement = XMMatrixTranslationFromVector(direction * Speed);

        _position = XMVector4Transform(_position, movement);
        _target = XMVector4Transform(_target, movement);

        // Rebuild the view matrix to reflect changes.
        _BuildView();
    }

    void Camera::Update(int xDelta, int yDelta)
    {
        float xRotationDelta = XMConvertToRadians((yDelta)*CAMERA_ROTATION_SPEED);
        float yRotationDelta = XMConvertToRadians((xDelta)*CAMERA_ROTATION_SPEED);

        XMMATRIX position = XMMatrixTranslationFromVector(_position);
        XMMATRIX invPosition = XMMatrixTranslationFromVector(XMVectorNegate(_position));
        XMMATRIX rotateAroundUp = XMMatrixRotationAxis(_up, yRotationDelta);
        XMMATRIX rotateAroundRight = XMMatrixRotationAxis(Right(), xRotationDelta);

        XMMATRIX positionTransform = invPosition * rotateAroundUp * rotateAroundRight * position;

        _target = XMVector4Transform(_target, positionTransform);
        //_up = XMVector3Transform(_up, positionTransform);

        // Rebuild the view matrix to reflect changes
        _BuildView();
    }

    const XMMATRIX& Camera::View() const
    {
        return _view;
    }

    const XMMATRIX& Camera::Projection() const
    {
        return _projection;
    }

    const XMMATRIX& Camera::ViewProjection() const
    {
        return _viewProjection;
    }

    XMVECTOR Camera::Right() const
    {
        return XMMatrixTranspose(_view).r[0];
    }

    XMVECTOR Camera::Up() const
    {
        return XMMatrixTranspose(_view).r[1];
    }

    XMVECTOR Camera::Look() const
    {
        return XMMatrixTranspose(_view).r[2];
    }

    const XMVECTOR& Camera::Position() const
    {
        return _position;
    }

    XMVECTOR& Camera::Position()
    {
        return _position;
    }

    const XMVECTOR& Camera::Target() const
    {
        return _target;
    }

    XMVECTOR& Camera::Target()
    {
        return _target;
    }

    void Camera::LookAt(const XMVECTOR& pos, const XMVECTOR& target, const XMVECTOR& up)
    {
        _position = pos;
        _target = target;
        _up = XMVector3Normalize(up);

        _BuildView();
    }

    const FrustumVolume& Camera::GetViewFrustum() const
    {
        return _frustum;
    }

    void Camera::SetViewport(const rhi::Viewport& viewport)
    {
        _viewport = viewport;

        _BuildProjection();
    }

    void Camera::SetLens(float fov, float nearZ, float farZ)
    {
        FoV = fov;
        NearZ = nearZ;
        FarZ = farZ;

        _BuildProjection();
    }

    void Camera::SetFoV(float fov)
    {
        FoV = fov;

        _BuildProjection();
    }

    float Camera::GetFoV() const
    {
        return FoV;
    }

    void Camera::SetNearZ(float nearZ)
    {
        NearZ = nearZ;

        _BuildProjection();
    }

    float Camera::GetNearZ() const
    {
        return NearZ;
    }

    void Camera::SetFarZ(float farZ)
    {
        FarZ = farZ;

        _BuildProjection();
    }

    float Camera::GetFarZ() const
    {
        return FarZ;
    }

    void Camera::SetSpeed(float speed)
    {
        Speed = speed;
    }

    float Camera::GetSpeed() const
    {
        return Speed;
    }

    rhi::Viewport Camera::GetViewport() const
    {
        return _viewport;
    }

    float Camera::GetAspectRatio() const
    {
        return _aspectRatio;
    }

    void Camera::SetSize(std::uint32_t width, std::uint32_t height)
    {
        _viewport.Width = (float)width;
        _viewport.Height = (float)height;
        _aspectRatio = (width / (float)height);

        _BuildProjection();
    }

    DirectX::XMUINT2 Camera::GetSize() const
    {
        return { (UINT)_viewport.Width, (UINT)_viewport.Height };
    }

    void Camera::SetDepth(const DirectX::XMFLOAT2& depth)
    {
        _viewport.MinDepth = depth.x;
        _viewport.MaxDepth = depth.y;
    }

    DirectX::XMFLOAT2 Camera::GetDepth() const
    {
        return { _viewport.MinDepth, _viewport.MaxDepth };
    }

    void Camera::SetScissorRectangle(const rhi::ScissorRect& rect)
    {
        _scissorRectangle = rect;
    }

    rhi::ScissorRect& Camera::GetScissorRectangle()
    {
        return _scissorRectangle;
    }

    const rhi::ScissorRect& Camera::GetScissorRectangle() const
    {
        return _scissorRectangle;
    }

    void Camera::_BuildView()
    {
        _view = XMMatrixLookAtLH(_position, _target, _up);

        _UpdateFrustum();
    }

    void Camera::_BuildProjection()
    {
        _projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(FoV), _aspectRatio, NearZ, FarZ);
        _UpdateFrustum();
    }

    void Camera::_UpdateFrustum()
    {
        _frustum.transform = _view;

        _viewProjection = _view * _projection;
        _frustum.BuildFromProjMatrix(_viewProjection);
    }
} // namespace scene
