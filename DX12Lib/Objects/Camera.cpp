#include "stdafx.h"

#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	_view = XMMatrixIdentity();
	_projection = XMMatrixIdentity();
	_viewProjection = XMMatrixIdentity();

	_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	_look = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	_right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	_target = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	_updateFrustum();

	// Client should adjust to a value that makes sense for application's
	// unit scale, and the object the camera is attached to--e.g., car, jet,
	// human walking, etc.
	_speed = 100.0f;

	_prevX = 1280; // TODO: whyyy???
	_prevY = 720;
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

XMVECTOR& Camera::Position()
{
	return _position;
}

void Camera::LookAt(XMVECTOR& pos, XMVECTOR& target, XMVECTOR& up)
{
	XMVECTOR L = XMVector3Normalize(target - pos);
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(up, L));
	XMVECTOR U = XMVector3Normalize(XMVector3Cross(L, R));

	_position = pos;
	_right = R;
	_up = up;
	_target = target;
	_look = L;

	_buildView();
}

void Camera::SetLens(float fov, float aspect, float nearZ, float farZ)
{
	_fov = fov;
	_aspectRatio = aspect;
	_nearZ = nearZ;
	_farZ = farZ;

	_buildProjection();
}

void Camera::SetFOV(float fov)
{
	_fov = fov;

	_buildProjection();
}

float Camera::GetFOV() const
{
	return _fov;
}

void Camera::SetAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;

	_buildProjection();
}

float Camera::GetAspectRatio() const
{
	return _aspectRatio;
}

void Camera::SetNearZ(float nearZ)
{
	_nearZ = nearZ;

	_buildProjection();
}

float Camera::GetNearZ() const
{
	return _nearZ;
}

void Camera::SetFarZ(float farZ)
{
	_farZ = farZ;

	_buildProjection();
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
	_position += direction * 0.5f;
	_target += direction * 0.5f;

	// Rebuild the view matrix to reflect changes.
	_buildView();
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
	_buildView();
	_prevX = xDelta;
	_prevY = yDelta;
}

void Camera::_buildView()
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
	_updateFrustum();
}

void Camera::_buildProjection()
{
	_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(_fov), _aspectRatio, _nearZ, _farZ);
	_updateFrustum();
}

void Camera::_updateFrustum()
{
	_frustum.buildFromProjMatrix(_view * _projection);
}
