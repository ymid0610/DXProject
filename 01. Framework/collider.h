#pragma once
#include "stdafx.h"
#include "mesh.h"

enum ColliderType
{
	BOX_TYPE,
	MESH_TYPE
};

class GameObject;

class Collider
{
public:
	// 생성자 소멸자
	Collider(ColliderType type) : m_type(type) {}
	~Collider() = default;

	// 업데이트 함수
	virtual void Update(const XMFLOAT4X4& worldMatrix) = 0;

	// Getter
	ColliderType GetType() const { return m_type; }
	virtual XMFLOAT3 GetCenter() const = 0;

	// Setter
	void SetOwner(const shared_ptr<GameObject>& owner) { m_owner = owner; }

	// 멤버	함수
	virtual bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const = 0;

public:
	weak_ptr<GameObject> m_owner;
		
protected:
	ColliderType m_type;
};

class BoxCollider : public Collider
{
public:
	BoxCollider(const shared_ptr<Mesh>& mesh) : Collider(ColliderType::BOX_TYPE)
	{
		if (mesh) {
			m_localOBB = mesh->GetLocalOBB(); // Mesh에서 추출
		}
	}

	// 업데이트 함수
	virtual void Update(const XMFLOAT4X4& worldMatrix) override
	{
		m_localOBB.Transform(m_worldOBB, XMLoadFloat4x4(&worldMatrix));
	}

	// Getter
	DirectX::BoundingOrientedBox GetWorldOBB() const { return m_worldOBB; }
	virtual XMFLOAT3 GetCenter() const override { return m_worldOBB.Center; }

	// Setter
	void SetOffset(XMFLOAT3 offset) { m_localOBB.Center = offset; }
	void SetScale(XMFLOAT3 scale) {}

	// 멤버 함수
	virtual bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outDist) const override;

private:
	DirectX::BoundingOrientedBox m_localOBB;
	DirectX::BoundingOrientedBox m_worldOBB;
};