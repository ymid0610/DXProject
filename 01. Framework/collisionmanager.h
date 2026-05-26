#pragma once

#include "stdafx.h"
#include "collider.h"

class Player;

struct CollisionEvent
{
	shared_ptr<Collider> colliderA;
	shared_ptr<Collider> colliderB;
};

struct ContactInfo {
	XMFLOAT3 normal;     // 충돌 표면의 법선
	float penetration;   // 두 물체가 겹친 깊이
};

class CollisionManager
{
public:
	// 생성자 소멸자
	CollisionManager() = default;
	~CollisionManager() = default;

	// 업데이트 함수
	void Update(const shared_ptr<Player>& player, FLOAT timeElapsed);

	// 충돌 이벤트 처리 함수
	void ProcessCollisions();

	// 멤버 함수
	void AddCollider(const shared_ptr<Collider>& collider);
	void ClearColliders();

	bool CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b, ContactInfo& outContact);
	bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist, 
		const shared_ptr<Collider>& ignoreCollider = nullptr) const;

private:
	vector<shared_ptr<Collider>> m_colliders;

	queue<CollisionEvent> m_eventQueue;
};