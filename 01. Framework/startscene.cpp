#include "scene.h"
#include "framework.h"

StartScene::StartScene()
{
}

void StartScene::BuildObjects(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature)
{
	m_shader = make_shared<Shader>(device, rootSignature);
	m_cube = make_shared<CubeIndexMesh>(device, commandList);

	m_player = make_shared<Player>();
	m_player->SetMesh(m_cube);
	m_player->SetPosition(XMFLOAT3{ 0.f, 0.f, 0.f });

	for (int x = -15; x <= 15; x += 5) {
		for (int y = -15; y <= 15; y += 5) {
			for (int z = -15; z <= 15; z += 5) {
				auto object = make_shared<RotatingObject>();
				object->SetMesh(m_cube);
				object->SetPosition(XMFLOAT3{
					static_cast<FLOAT>(x),
					static_cast<FLOAT>(y),
					static_cast<FLOAT>(z) });
				m_objects.push_back(object);
			}
		}
	}

	m_camera = make_shared<SpringArmCamera>();
	m_camera->SetLens(0.25 * XM_PI, g_framework->GetAspectRatio(), 0.1f, 1000.f);

	m_player->SetCamera(m_camera);
}