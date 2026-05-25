#pragma once
#include "stdafx.h"
#include "shader.h"
#include "mesh.h"
#include "object.h"
#include "player.h"
#include "camera.h"
#include "collisionmanager.h"

class Scene
{
public:
	// 생성자 소멸자
	Scene();
	~Scene() = default;

	// 업데이트 함수
	virtual void Update(FLOAT timeElapsed);

	// 렌더링 함수
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	// 멤버	함수
	virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12RootSignature>& rootSignature);
	void ReleaseObjects();
	void ReleaseUploadBuffer();

	// 입력 이벤트 처리 함수
	void MouseEvent(HWND hWnd, FLOAT timeElapsed);
	void KeyboardEvent(FLOAT timeElapsed);
	void MouseEvent(UINT message, LPARAM lParam);
	void KeyboardEvent(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void MouseEvent(UINT message, WPARAM wParam, LPARAM lParam);

protected:
	shared_ptr<Shader> m_shader;
	shared_ptr<Camera> m_camera;

	shared_ptr<Player> m_player;
	vector<shared_ptr<GameObject>> m_objects;

	shared_ptr<Mesh> m_cube;
	unique_ptr<CollisionManager> m_collisionManager;
};

class StartScene : public Scene
{
public:
	// 생성자 소멸자
	StartScene();
	~StartScene() = default;

	// 업데이트 함수
	virtual void Update(FLOAT timeElapsed);

	// 멤버	함수
	virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12RootSignature>& rootSignature);
};