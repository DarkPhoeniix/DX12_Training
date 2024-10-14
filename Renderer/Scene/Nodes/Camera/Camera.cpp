#include "stdafx.h"

#include "Camera.h"

using namespace DirectX;

namespace
{
	constexpr float CAMERA_MOVEMENT_SPEED = 0.1f;
} // namespace unnamed

namespace SceneLayer
{
	Camera::Camera()
		: Base()
		, _view(XMMatrixIdentity())
		, _projection(XMMatrixIdentity())
		, _viewProjection(XMMatrixIdentity())
		, _position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f))
		, _right(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f))
		, _up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
		, _look(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f))
		, _viewport()
		, _speed(100.0f)
		, _prevX(1280)
		, _prevY(720)
	{
		_UpdateFrustum();
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
		return _view * _projection;
	}

	const XMVECTOR& Camera::Right() const
	{
		return _right;
	}

	const XMVECTOR& Camera::Up() const
	{
		return _up;
	}

	const XMVECTOR& Camera::Look() const
	{
		return _look;
	}

	const FrustumVolume& Camera::GetViewFrustum() const
	{
		return _frustum;
	}

	void Camera::LookAt(XMVECTOR& pos, XMVECTOR& target, XMVECTOR& up)
	{
		XMVECTOR L = XMVector3Normalize(target - pos);
		XMVECTOR R = XMVector3Normalize(XMVector3Cross(up, L));
		XMVECTOR U = XMVector3Normalize(XMVector3Cross(L, R));

		_position = pos;
		_right = R;
		_up = up;
		_look = L;

		_BuildView();
	}

	void Camera::SetViewport(const Viewport& viewport)
	{
		_viewport = viewport;
	}

	Viewport Camera::GetViewport() const
	{
		return _viewport;
	}

	CD3DX12_VIEWPORT Camera::GetDXViewport() const
	{
		return _viewport.GetDXViewport();
	}

	CD3DX12_RECT Camera::GetDXScissorRectangle() const
	{
		return _viewport.GetScissorRectangle();
	}

	void Camera::SetLens(float fov, float nearZ, float farZ)
	{
		_fov = fov;
		_nearZ = nearZ;
		_farZ = farZ;

		_BuildProjection();
	}

	void Camera::SetFOV(float fov)
	{
		_fov = fov;

		_BuildProjection();
	}

	float Camera::GetFOV() const
	{
		return _fov;
	}

	void Camera::SetNearZ(float nearZ)
	{
		_nearZ = nearZ;

		_BuildProjection();
	}

	float Camera::GetNearZ() const
	{
		return _nearZ;
	}

	void Camera::SetFarZ(float farZ)
	{
		_farZ = farZ;

		_BuildProjection();
	}

	float Camera::GetFarZ() const
	{
		return _farZ;
	}

	void Camera::SetSpeed(float s)
	{
		_speed = s;
	}

	float Camera::GetSpeed() const
	{
		return _speed;
	}

	void Camera::Update(XMVECTOR direction)
	{
		_position += direction * CAMERA_MOVEMENT_SPEED;

		// Rebuild the view matrix to reflect changes.
		_BuildView();
	}

	void Camera::Update(int xDelta, int yDelta)
	{
		float xRotationDelta = -XMConvertToRadians((-yDelta) / 5.5f);
		float yRotationDelta = -XMConvertToRadians((-xDelta) * 5.5f);

		// Rotate camera's look and up vectors around the camera's right vector.
		XMMATRIX R = XMMatrixRotationAxis(_right, xRotationDelta);
		_look = XMVector3TransformCoord(_look, R);
		_up = XMVector3TransformCoord(_up, R);


		// Rotate camera axes about the world's y-axis.
		XMMATRIX U = XMMatrixRotationY(XMConvertToRadians(yRotationDelta));
		_right = XMVector3Transform(_right, U);
		_up = XMVector3Transform(_up, U);
		_look = XMVector3Transform(_look, U);

		// Rebuild the view matrix to reflect changes
		_BuildView();
		_prevX = xDelta;
		_prevY = yDelta;
	}

	void Camera::_BuildView()
	{
		// Keep camera's axes orthogonal to each other and of unit length.
		_look = XMVector3Normalize(_look);
		_right = XMVector3Normalize(XMVector3Cross(_up, _look));
		_up = XMVector3Normalize(XMVector3Cross(_look, _right));

		// Fill in the view matrix entries.

		float x = -XMVectorGetX(XMVector3Dot(_position, _right));
		float y = -XMVectorGetY(XMVector3Dot(_position, _up));
		float z = -XMVectorGetZ(XMVector3Dot(_position, _look));

		_view = XMMatrixTranspose(XMMatrixSet(XMVectorGetX(_right), XMVectorGetY(_right), XMVectorGetZ(_right), x,
			XMVectorGetX(_up), XMVectorGetY(_up), XMVectorGetZ(_up), y,
			XMVectorGetX(_look), XMVectorGetY(_look), XMVectorGetZ(_look), z,
			0.0f, 0.0f, 0.0f, 1.0f));

		_frustum.transform = _view;
		_UpdateFrustum();
	}

	void Camera::_BuildProjection()
	{
		_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(_fov), _viewport.GetAspectRatio(), _nearZ, _farZ);
		_UpdateFrustum();
	}

	void Camera::_UpdateFrustum()
	{
		_frustum.BuildFromProjMatrix(_view * _projection);
	}
} // namespace SceneLayer
