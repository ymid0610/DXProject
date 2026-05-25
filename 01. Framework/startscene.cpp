#include "scene.h"
#include "framework.h"
#include "collisionmanager.h"

StartScene::StartScene()
{
}

// 업데이트 함수
void StartScene::Update(FLOAT timeElapsed)
{
	Scene::Update(timeElapsed);

	auto springCamera = dynamic_pointer_cast<SpringArmCamera>(m_camera);
	if (springCamera && m_collisionManager)
	{
		XMFLOAT3 origin = springCamera->GetLookAtPosition();
		XMFLOAT3 direction = springCamera->GetDirectionToCamera();
		float maxDistance = springCamera->GetArmLength();
		float hitDist = 0.0f;

		if (m_collisionManager->Raycast(origin, direction, hitDist, m_player->GetCollider()))
		{
			if (hitDist < maxDistance)
			{
				springCamera->SetCollisionDistance(hitDist);
			}
		}
	}
}

void StartScene::BuildObjects(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
	m_collisionManager = make_unique<CollisionManager>();

	m_shader = make_shared<Shader>(device, rootSignature);
	m_cube = make_shared<CubeIndexMesh>(device, commandList);

	m_player = make_shared<Player>();
	m_player->SetMesh(m_cube);
	m_player->SetPosition(XMFLOAT3{ 0.f, 0.f, 0.f });

	auto playerCollider = make_shared<BoxCollider>(m_cube);
	m_player->SetCollider(playerCollider);

	m_collisionManager->AddCollider(playerCollider);

	for (int x = -15; x <= 15; x += 5) {
		for (int y = -15; y <= 15; y += 5) {
			for (int z = -15; z <= 15; z += 5) {

				if(x == 0 && y == 0 && z == 0) continue;

				auto object = make_shared<RotatingObject>();
				object->SetMesh(m_cube);
				object->SetPosition(XMFLOAT3{
					static_cast<FLOAT>(x),
					static_cast<FLOAT>(y),
					static_cast<FLOAT>(z) });

				auto objCollider = make_shared<BoxCollider>(m_cube);
				object->SetCollider(objCollider);

				m_objects.push_back(object);

				m_collisionManager->AddCollider(objCollider);
			}
		}
	}

	m_camera = make_shared<SpringArmCamera>();
	m_camera->SetLens(0.25 * XM_PI, g_framework->GetAspectRatio(), 0.1f, 1000.f);

	m_player->SetCamera(m_camera);
}