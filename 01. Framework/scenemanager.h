#pragma once
#include "stdafx.h"
#include "scene.h"

class SceneManager
{
public:
	// 생성자 소멸자
	SceneManager();
	~SceneManager() = default;

	// 업데이트 함수
	void Update(FLOAT timeElapsed);

	// 렌더링 함수
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void ReleaseUploadBuffer();
	
	// 멤버 함수
	void ChangeScene(const ComPtr<ID3D12Device>& device, 
		const ComPtr<ID3D12GraphicsCommandList>& commandList, 
		const ComPtr<ID3D12RootSignature>& rootSignature, std::unique_ptr<Scene> newScene);
	void PushScene(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12RootSignature>& rootSignature, std::unique_ptr<Scene> newScene);
	void PopScene();

	// 입력 이벤트 처리 함수
	void MouseEvent(HWND hWnd, FLOAT timeElapsed);
	void KeyboardEvent(FLOAT timeElapsed);
	void MouseEvent(UINT message, LPARAM lParam);
	void MouseEvent(UINT message, WPARAM wParam, LPARAM lParam);
	void KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	vector<std::unique_ptr<Scene>> m_scenes;
};

