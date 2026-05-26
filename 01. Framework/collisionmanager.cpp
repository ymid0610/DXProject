#include "collisionmanager.h"
#include "player.h"

void CollisionManager::AddCollider(const shared_ptr<Collider>& collider)
{
	if (collider) {
		m_colliders.push_back(collider);
	}
}

void CollisionManager::ClearColliders()
{
	m_colliders.clear();
}

void CollisionManager::Update(const shared_ptr<Player>& player, FLOAT timeElapsed)
{
	for (size_t i = 0; i < m_colliders.size(); ++i) {
		for (size_t j = i + 1; j < m_colliders.size(); ++j) {
			auto colA = m_colliders[i];
			auto colB = m_colliders[j];
			if (!colA || !colB) continue;

			ContactInfo contact;
			if (CheckCollision(colA, colB, contact)) {
				auto objA = colA->m_owner.lock();
				auto objB = colB->m_owner.lock();
				if (!objA || !objB) continue;

				float invMassA = objA->GetInverseMass();
				float invMassB = objB->GetInverseMass();
				float invMassSum = invMassA + invMassB;

				if (invMassSum > 0.0f) {

					XMFLOAT3 velA = objA->GetVelocity();
					XMFLOAT3 velB = objB->GetVelocity();
					XMFLOAT3 relativeVel = Utiles::Vector3::Sub(velB, velA);

					float velAlongNormal = Utiles::Vector3::Dot(relativeVel, contact.normal);

					float e = min(objA->GetRestitution(), objB->GetRestitution());

					// 바움가르테 안정화 (Baumgarte Stabilization)
					// 위치 보정을 '물리적인 밀어내기 속도'로 
					const float baumgarte_slop = 0.01f;
					const float baumgarte_percent = 0.2f; 

					// 겹친 깊이에 비례한 추가 분리 속도 목표치 (Resting Contact 시 떨림 방지)
					float bias = max(contact.penetration - baumgarte_slop, 0.0f) * baumgarte_percent / timeElapsed;

					// 만약 밀착 상태(속도가 거의 0인 Resting 상태)면 통통 튀는(Restitution) 것을 무시 
					if (abs(velAlongNormal) < 0.1f) {
						e = 0.0f; // 마이크로 바운싱(진동) 방지
					}

					// 충격량 크기(j) 계산 공식에 Bias(밀어내기 여유분 속도) 합산
					float j = -(1.0f + e) * velAlongNormal + bias;
					j /= invMassSum;

					// 밀어내는 방향(Impulse) 도출
					XMFLOAT3 impulse = Utiles::Vector3::Mul(contact.normal, j);

					// 더 이상 Transform()으로 위치를 억지로 끄집어내지 않고,
					// 정확한 물리 속도를 가해 자연스럽게 튕겨져 나오게 함
					if (invMassA > 0.0f) {
						objA->AddImpulse(Utiles::Vector3::Mul(impulse, -1.0f));
					}
					if (invMassB > 0.0f) {
						objB->AddImpulse(impulse);
					}
				}

				m_eventQueue.push({ colA, colB });
			}
		}
	}


	ProcessCollisions();
}

void CollisionManager::ProcessCollisions()
{
	while (!m_eventQueue.empty())
	{
		const auto& event = m_eventQueue.front();
		m_eventQueue.pop();

		// 안전하게 weak_ptr(lock)를 사용해 살아있는지 확인 후 로직(콜백) 실행
		if (auto ownerA = event.colliderA->m_owner.lock()) {
			ownerA->OnCollisionEnter(event.colliderB);
		}
		if (auto ownerB = event.colliderB->m_owner.lock()) {
			ownerB->OnCollisionEnter(event.colliderA);
		}
	}
}

// OBB 충돌 체크 및 방향/깊이 근사 추출
bool CollisionManager::CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b, ContactInfo& outContact)
{
	if (!a || !b) return false;

	if (a->GetType() == ColliderType::BOX_TYPE && b->GetType() == ColliderType::BOX_TYPE) {
		auto boxA = static_pointer_cast<BoxCollider>(a);
		auto boxB = static_pointer_cast<BoxCollider>(b);

		const auto& obbA = boxA->GetWorldOBB();
		const auto& obbB = boxB->GetWorldOBB();

		if (obbA.Intersects(obbB)) {
			// Center 기준 거리 계산
			XMFLOAT3 delta = Utiles::Vector3::Sub(obbB.Center, obbA.Center);

			// 각 축(X, Y, Z)별로 겹친 깊이구하기
			float overlapX = (obbA.Extents.x + obbB.Extents.x) - abs(delta.x);
			float overlapY = (obbA.Extents.y + obbB.Extents.y) - abs(delta.y);
			float overlapZ = (obbA.Extents.z + obbB.Extents.z) - abs(delta.z);

			// 세 축 중 '가장 조금 겹친 축'이 실제 충돌이 발생한 면(Face)
			if (overlapX < overlapY && overlapX < overlapZ) {
				outContact.normal = delta.x > 0 ? XMFLOAT3(1, 0, 0) : XMFLOAT3(-1, 0, 0);
				outContact.penetration = overlapX;
			}
			else if (overlapY < overlapX && overlapY < overlapZ) {
				outContact.normal = delta.y > 0 ? XMFLOAT3(0, 1, 0) : XMFLOAT3(0, -1, 0);
				outContact.penetration = overlapY;
			}
			else {
				outContact.normal = delta.z > 0 ? XMFLOAT3(0, 0, 1) : XMFLOAT3(0, 0, -1);
				outContact.penetration = overlapZ;
			}

			// 안전 장치: 거리가 완벽히 0히면 임의로 위로 튕겨줌
			if (outContact.penetration <= 0.0f) {
				outContact.normal = XMFLOAT3(0, 1, 0);
				outContact.penetration = 0.01f;
			}

			return true;
		}
	}
	return false;
}

bool CollisionManager::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist, const shared_ptr<Collider>& ignoreCollider) const
{
	bool isHit = false;
	float minHitDist = FLT_MAX;

	for (const auto& collider : m_colliders)
	{
		if (!collider || collider == ignoreCollider) continue;

		float hitDist = 0.0f;
		if (collider->Raycast(origin, direction, hitDist))
		{
			if (hitDist < minHitDist)
			{
				minHitDist = hitDist;
				isHit = true;
			}
		}
	}

	if (isHit)
	{
		outHitDist = minHitDist;
	}

	return isHit;
}