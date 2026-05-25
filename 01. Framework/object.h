#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "collider.h"

class GameObject
{
public:
	// 생성자 소멸자
	GameObject();
	~GameObject() = default;

	// 업데이트 함수
	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	// Getter
	shared_ptr<Collider> GetCollider() const { return m_collider; }
	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const { return XMFLOAT3{m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43}; }

	// Setter
	void SetMesh(const shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
	void SetPosition(XMFLOAT3 position) {
		m_worldMatrix._41 = position.x;
		m_worldMatrix._42 = position.y;
		m_worldMatrix._43 = position.z;
	}
	void SetCollider(const shared_ptr<Collider>& collider) { m_collider = collider; }
	void SetPreviousPosition(XMFLOAT3 position) { m_previousPosition = position; }

	// 멤버 함수
	void Transform(XMFLOAT3 shift);
	void Rotate(FLOAT pitch, FLOAT yaw, FLOAT roll);
	void RevertPosition() { SetPosition(m_previousPosition); }

protected:
	XMFLOAT4X4					m_worldMatrix;

	XMFLOAT3					m_right;
	XMFLOAT3					m_up;
	XMFLOAT3					m_front;

	XMFLOAT3					m_previousPosition;
	shared_ptr<Mesh>			m_mesh;

	shared_ptr<Collider>		m_collider;
};

class RotatingObject : public GameObject
{
public:
	RotatingObject();
	~RotatingObject() = default;

	void Update(FLOAT timeElapsed) override;

private:
	FLOAT m_rotatingSpeed;
};
