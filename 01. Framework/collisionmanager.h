#pragma once

#include "stdafx.h"
#include "collider.h"

class Player;

class CollisionManager
{
public:
	// 생성자 소멸자
	CollisionManager() = default;
	~CollisionManager() = default;

	// 업데이트 함수
	void Update(const shared_ptr<Player>& player, FLOAT timeElapsed);

	// 멤버 함수
	void AddCollider(const shared_ptr<Collider>& collider);
	void ClearColliders();

	bool CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b);
	bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist, 
		const shared_ptr<Collider>& ignoreCollider = nullptr) const;

private:
	vector<shared_ptr<Collider>> m_colliders;
};