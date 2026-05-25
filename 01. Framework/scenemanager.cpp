#include "scenemanager.h"

SceneManager::SceneManager()
{
}

void SceneManager::Update(FLOAT timeElapsed)
{
	if (!m_scenes.empty()) {
		m_scenes.back()->Update(timeElapsed);
	}
}

void SceneManager::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (!m_scenes.empty()) {
		m_scenes.back()->Render(commandList);
	}
}

void SceneManager::ChangeScene(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList, 
	const ComPtr<ID3D12RootSignature>& rootSignature, std::unique_ptr<Scene> newScene)
{
	if (!m_scenes.empty()) {
		m_scenes.pop_back();
	}
	m_scenes.emplace_back(std::move(newScene));
	m_scenes.back()->BuildObjects(device, commandList, rootSignature);
}

void SceneManager::PushScene(const ComPtr<ID3D12Device>& device,
	const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const ComPtr<ID3D12RootSignature>& rootSignature, std::unique_ptr<Scene> newScene)
{
	m_scenes.emplace_back(std::move(newScene));
	m_scenes.back()->BuildObjects(device, commandList, rootSignature);
}

void SceneManager::PopScene()
{
	if (!m_scenes.empty()) {
		m_scenes.pop_back();
	}
}

void SceneManager::ReleaseUploadBuffer()
{
	if (!m_scenes.empty()) {
		m_scenes.back()->ReleaseUploadBuffer();
	}
}

void SceneManager::MouseEvent(HWND hWnd, FLOAT timeElapsed)
{
	if (!m_scenes.empty()) {
		m_scenes.back()->MouseEvent(hWnd, timeElapsed);
	}
}

void SceneManager::KeyboardEvent(FLOAT timeElapsed)
{
	if (!m_scenes.empty()) {
		m_scenes.back()->KeyboardEvent(timeElapsed);
	}
}

void SceneManager::MouseEvent(UINT message, LPARAM lParam)
{
	if (!m_scenes.empty()) {
		m_scenes.back()->MouseEvent(message, lParam);
	}
}

void SceneManager::MouseEvent(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!m_scenes.empty()) {
		m_scenes.back()->MouseEvent(message, wParam, lParam);
	}
}

void SceneManager::KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ESC 키가 눌렸을 때 (WM_KEYDOWN)
	if (message == WM_KEYDOWN && wParam == VK_ESCAPE)
	{
		// 씬 팝 (이전 씬으로 돌아감)
		PopScene();
		
		// 팝 이후 남은 씬이 없다면 프로그램을 종료할지 결정
		if (m_scenes.empty()) {
			PostQuitMessage(0);
		}
		
		return; // 추가적인 키 이벤트를 아래 씬으로 넘기지 않음
	}

	// 그 외의 키 입력은 현재 최상단 씬에게 전달
	if (!m_scenes.empty()) {
		m_scenes.back()->KeyboardEvent(hWnd, message, wParam, lParam);
	}
}

