#pragma once

#include "Scene/Entity/Components/IComponent.h"
#include "Scene/Volumes/FrustumVolume.h"

namespace scene
{
	class Viewport
	{
	public:
		Viewport();
		Viewport(const DirectX::XMUINT2& size);
		Viewport(const CD3DX12_VIEWPORT& DXViewport);
		~Viewport() = default;

		CD3DX12_VIEWPORT GetDXViewport() const;
		float GetAspectRatio() const;

		void SetSize(const DirectX::XMUINT2& size);
		DirectX::XMUINT2 GetSize() const;

		void SetDepth(const DirectX::XMFLOAT2& depth);
		DirectX::XMFLOAT2 GetDepth() const;

		void SetScissorRectangle(const CD3DX12_RECT& rect);
		CD3DX12_RECT& GetScissorRectangle();
		const CD3DX12_RECT& GetScissorRectangle() const;

	private:
		CD3DX12_VIEWPORT _viewport;
		CD3DX12_RECT _scissorRectangle;
		float _aspectRatio;
	};

	class Camera : public IComponent
	{
	public:
		Camera();
		~Camera() = default;

		void Update();
		void Update(DirectX::XMVECTOR direction);
		void Update(int pitch, int yaw);

		const DirectX::XMMATRIX& View() const;
		const DirectX::XMMATRIX& Projection() const;
		const DirectX::XMMATRIX& ViewProjection() const;

		const DirectX::XMVECTOR& Right() const;
		const DirectX::XMVECTOR& Up() const;
		const DirectX::XMVECTOR& Look() const;

		const DirectX::XMVECTOR& Position() const;
		DirectX::XMVECTOR& Position();
		const DirectX::XMVECTOR& Target() const;
		DirectX::XMVECTOR& Target();

		void LookAt(const DirectX::XMVECTOR& pos, const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& up);

		const FrustumVolume& GetViewFrustum() const;

		void SetViewport(const Viewport& viewport);
		Viewport& GetViewport();

		void SetLens(float fov, float nearZ, float farZ);

		// Lens params
		float FoV;
		float NearZ;
		float FarZ;

		// Camera speed
		float Speed;

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

		Viewport _viewport;
	};
} // namespace scene
