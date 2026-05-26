#include "scene.h"
#include "stdafx.h"
#include "framework.h"
#include "collisionmanager.h"

Scene::Scene()
{
}

// 업데이트	함수
void Scene::Update(FLOAT timeElapsed)
{
	if (m_player) m_player->Update(timeElapsed);

	if (!m_objects.empty()) {
		for (auto& object : m_objects) {
			object->Update(timeElapsed);
		}
	}

	if (m_physicsManager)
	{
		m_physicsManager->Update(timeElapsed);
	}

	if (m_collisionManager)
	{
		m_collisionManager->Update(m_player, timeElapsed);
	}
}

void Scene::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
	SetCursor(NULL);
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	POINT lastMousePosition{
		windowRect.left + static_cast<LONG>(g_framework->GetWindowWidth() / 2),
		windowRect.top + static_cast<LONG>(g_framework->GetWindowHeight() / 2) };
	POINT mousePosition;
	GetCursorPos(&mousePosition);

	float dx = XMConvertToRadians(0.15f * static_cast<FLOAT>(lastMousePosition.x - mousePosition.x));
	float dy = XMConvertToRadians(0.15f * static_cast<FLOAT>(lastMousePosition.y - mousePosition.y));

	if (m_camera) {
		auto springCamera = dynamic_pointer_cast<SpringArmCamera>(m_camera);

		// 1인칭 모드 판별 (ArmLength가 0에 근접한 경우)
		if (springCamera && springCamera->GetArmLength() <= 1.0f) {
			// 플레어어 캐릭터 모델 자체를 마우스 X축 회전(dx)만큼 Y축으로 회전시킵니다.
			m_player->Rotate(0.0f, XMConvertToDegrees(-dx), 0.0f);
			m_camera->RotateYaw(dx);
		}

		m_camera->RotateYaw(dx);
		m_camera->RotatePitch(dy);
	}
	SetCursorPos(lastMousePosition.x, lastMousePosition.y);

	m_player->MouseEvent(timeElapsed, 0);
}

void Scene::KeyboardEvent(FLOAT timeElapsed)
{
	m_player->SetPreviousPosition(m_player->GetPosition());

	m_player->KeyboardEvent(timeElapsed);
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	m_camera->UpdateShaderVariable(commandList);
	m_shader->UpdateShaderVariable(commandList);
	for (auto& object : m_objects) {
		object->Render(commandList);
	}
	m_player->Render(commandList);
}

void Scene::BuildObjects(const ComPtr<ID3D12Device>& device, 
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

	m_camera = make_shared<ThirdPersonCamera>();
	m_camera->SetLens(0.25 * XM_PI, g_framework->GetAspectRatio(), 0.1f, 1000.f);
	m_player->SetCamera(m_camera);
}

void Scene::ReleaseObjects()
{
	m_objects.clear();
	m_player.reset();
	m_camera.reset();
	m_shader.reset();
	m_cube.reset();
}

void Scene::ReleaseUploadBuffer()
{
	m_cube->ReleaseUploadBuffer();

}

void Scene::MouseEvent(UINT message, LPARAM lParam)
{
}

void Scene::KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
}

void Scene::MouseEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MOUSEWHEEL)
	{
		// 120 단위로 값이 들어옴 (위로 굴리면 양수, 아래로 굴리면 음수)
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		// 플레이어에게 휠 값을 전달 (함수 시그니처 변경 필요)
		m_player->MouseEvent(0.0f, wheelDelta);
	}
}