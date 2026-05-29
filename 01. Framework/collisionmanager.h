#pragma once

#include "stdafx.h"
#include "collider.h"

struct CollisionEvent
{
    shared_ptr<Collider> colliderA;
    shared_ptr<Collider> colliderB;
};

struct ContactInfo
{
    XMFLOAT3 normal{};
    float penetration = 0.0f;
};

class CollisionManager
{
public:
    CollisionManager() = default;
    ~CollisionManager() = default;

    void Update();
    void AddCollider(const shared_ptr<Collider>& collider);
    void ClearColliders();

    bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist,
        const shared_ptr<Collider>& ignoreCollider = nullptr) const;

private:
    bool CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b, ContactInfo& outContact);
    void ProcessCollisions();

private:
    vector<shared_ptr<Collider>> m_colliders;
    queue<CollisionEvent> m_eventQueue;
};