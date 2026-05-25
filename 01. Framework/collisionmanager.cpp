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
	if (!player || !player->GetCollider()) return;

	auto playerCollider = player->GetCollider();

	for (const auto& collider : m_colliders)
	{
		// 플레이어 자신과의 충돌 검사는 건너뜁니다.
		if (collider == playerCollider) continue;

		// 1차 OBB 충돌 검사
		if (CheckCollision(playerCollider, collider))
		{
			// 이동하기 전의 이전 위치로 플레이어를 되돌림
			player->RevertPosition();
			player->Update(0.0f); // 바뀐 위치로 플레이어 OBB 즉시 동기화

			// 2차 검사: 회전하는 벽이 가만히 서 있는 플레이어를 덮쳤을 때 (끼임 방지 밀어내기)
			if (CheckCollision(player->GetCollider(), collider))
			{
				XMFLOAT3 objCenter = collider->GetCenter();
				XMFLOAT3 playerPos = player->GetPosition();

				// 장애물 중심 -> 플레이어 방향 벡터 계산
				XMFLOAT3 pushDir = Utiles::Vector3::Sub(playerPos, objCenter);
				pushDir.y = 0.0f; // Y축 날아뜀 방지
				pushDir = Utiles::Vector3::Normalize(pushDir);

				// 밀어내는 속도 감도 조정
				float pushSpeed = 15.0f;

				// 안전한 외곽 방향으로 강제 위치 이동
				player->Transform(Utiles::Vector3::Mul(pushDir, pushSpeed * timeElapsed));
				player->Update(0.0f); // 밀려난 위치로 최종 동기화
			}
			break;
		}
	}
}

bool CollisionManager::CheckCollision(const shared_ptr<Collider>& a, const shared_ptr<Collider>& b)
{
	if (!a || !b) return false;

	if (a->GetType() == ColliderType::BOX_TYPE && b->GetType() == ColliderType::BOX_TYPE) {
		auto boxA = static_pointer_cast<BoxCollider>(a);
		auto boxB = static_pointer_cast<BoxCollider>(b);
		return boxA->GetWorldOBB().Intersects(boxB->GetWorldOBB());
	}
	return false;
}

bool CollisionManager::Raycast(const XMFLOAT3& origin, const XMFLOAT3& direction, float& outHitDist, const shared_ptr<Collider>& ignoreCollider) const
{
	bool isHit = false;
	float minHitDist = FLT_MAX;

	for (const auto& collider : m_colliders)
	{
		// 🌟 추가: 콜라이더가 비어있거나, '무시할 콜라이더'와 같으면 건너뜁니다.
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