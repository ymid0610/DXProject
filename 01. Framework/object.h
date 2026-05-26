#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "collider.h"
#include "rigidbody.h"

class GameObject : public std::enable_shared_from_this<GameObject>
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
	shared_ptr<Rigidbody> GetRigidbody() const { return m_rigidbody; }
	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const { return XMFLOAT3{m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43}; }

	virtual float GetInverseMass() const {
		if (m_rigidbody) {
			return m_rigidbody->IsKinematic() ? 0.0f : 1.0f / m_rigidbody->GetMass();
		}
		return 0.0f;
	}
	virtual float GetRestitution() const { return m_rigidbody ? m_rigidbody->GetRestitution() : 0.0f; }
	virtual XMFLOAT3 GetVelocity() const { return m_rigidbody ? m_rigidbody->GetVelocity() : XMFLOAT3{ 0.f, 0.f, 0.f }; }

	// Setter
	void SetMesh(const shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
	void SetPosition(XMFLOAT3 position) {
		m_worldMatrix._41 = position.x;
		m_worldMatrix._42 = position.y;
		m_worldMatrix._43 = position.z;
	}
	void SetPreviousPosition(XMFLOAT3 position) { m_previousPosition = position; }

	void SetCollider(const shared_ptr<Collider>& collider)
	{
		m_collider = collider;
		m_collider->SetOwner(shared_from_this()); 
	}
	void SetRigidbody(const shared_ptr<Rigidbody>& rigidbody)
	{
		m_rigidbody = rigidbody;
		m_rigidbody->SetOwner(shared_from_this());
	}

	// 멤버 함수
	void Transform(XMFLOAT3 shift);
	void Rotate(FLOAT pitch, FLOAT yaw, FLOAT roll);
	void RevertPosition() { SetPosition(m_previousPosition); }

	virtual void OnCollisionEnter(const shared_ptr<Collider>& other) {}

	virtual void AddImpulse(XMFLOAT3 impulse)
	{
		if (m_rigidbody && !m_rigidbody->IsKinematic()) {
			m_rigidbody->AddForce(impulse, ForceMode::Impulse);
		}
	}

protected:
	XMFLOAT4X4					m_worldMatrix;

	XMFLOAT3					m_right;
	XMFLOAT3					m_up;
	XMFLOAT3					m_front;

	XMFLOAT3					m_previousPosition;
	shared_ptr<Mesh>			m_mesh;

	shared_ptr<Collider>		m_collider;
	shared_ptr<Rigidbody>		m_rigidbody;
};

class RotatingObject : public GameObject
{
public:
	RotatingObject();
	~RotatingObject() = default;

	void Update(FLOAT timeElapsed) override;

	virtual void OnCollisionEnter(const shared_ptr<Collider>& other) override;

private:
	FLOAT m_rotatingSpeed;
};
