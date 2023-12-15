#include "stdafx.h"

#include "Camera.h"

using namespace DirectX;

Camera::Camera()
{
	_view = XMMatrixIdentity();
	_projection = XMMatrixIdentity();
	_viewProjection = XMMatrixIdentity();

	_position = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	_right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	_up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	_target = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// Client should adjust to a value that makes sense for application's
	// unit scale, and the object the camera is attached to--e.g., car, jet,
	// human walking, etc.
	_speed = 100.0f;

	_prevX = 1280;
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
	return _viewProjection;
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

	buildView();

	//_view = XMMatrixLookAtLH(pos, target, up);

	_viewProjection = _view * _projection;
}

void Camera::SetLens(float fov, float aspect, float nearZ, float farZ)
{
	_projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), aspect, nearZ, farZ);
	_viewProjection = _view * _projection;
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
	// Find the net direction the camera is traveling in (since the
	// camera could be running and strafing).
	//XMVECTOR dir(0.0f, 0.0f, 0.0f, 0.0f);
	//if (gDInput->keyDown(DIK_W))
	//	dir += mLookW;
	//if (gDInput->keyDown(DIK_S))
	//	dir -= mLookW;
	//if (gDInput->keyDown(DIK_D))
	//	dir += mRightW;
	//if (gDInput->keyDown(DIK_A))
	//	dir -= mRightW;

	//// Move at mSpeed along net direction.
	//dir = XMVector3Normalize(dir);
	//mPosW += dir * mSpeed * dt;


	//// We rotate at a fixed speed.
	//float pitch = gDInput->mouseDY() / 150.0f;
	//float yAngle = gDInput->mouseDX() / 150.0f;


	//// Rotate camera's look and up vectors around the camera's right vector.
	//XMMATRIX R = XMMatrixRotationAxis(mRightW, pitch);
	//mLookW = XMVector3TransformCoord(mLookW, R);
	//mUpW = XMVector3TransformCoord(mUpW, R);


	//// Rotate camera axes about the world's y-axis.
	//R = XMMatrixRotationY(yAngle);
	//mRightW = XMVector3TransformCoord(mRightW, R);
	//mUpW = XMVector3TransformCoord(mUpW, R);
	//mLookW = XMVector3TransformCoord(mLookW, R);

	// Rebuild the view matrix to reflect changes.
	//LookAt(_position, _target, _up);
	buildView();
}

void Camera::Update(int pitch, int yaw)
{
	float xDelta = -XMConvertToRadians((-yaw) / 15.0f);
	float yDelta = -XMConvertToRadians((-pitch) * 2.0f);
	// Rotate camera's look and up vectors around the camera's right vector.
	XMMATRIX R = XMMatrixRotationAxis(_right, xDelta);
	_look = XMVector3TransformCoord(_look, R);
	_up = XMVector3TransformCoord(_up, R);


	// Rotate camera axes about the world's y-axis.
	XMMATRIX U = XMMatrixRotationY(XMConvertToRadians(yDelta));
	_right = XMVector3Transform(_right, U);
	_up = XMVector3Transform(_up, U);
	_look = XMVector3Transform(_look, U);

	// Rebuild the view matrix to reflect changes.
	//LookAt(_position, _target, _up);
	buildView();
	_prevX = pitch;
	_prevY = yaw;
}

void Camera::buildView()
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
}
