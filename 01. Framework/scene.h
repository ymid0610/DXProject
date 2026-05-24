#pragma once
#include "stdafx.h"
#include "shader.h"
#include "mesh.h"
#include "object.h"
#include "player.h"
#include "camera.h"

class Scene
{
public:
	Scene();
	~Scene() = default;

	virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12RootSignature>& rootSignature);
	void ReleaseObjects();

	void ReleaseUploadBuffer();

	void Update(FLOAT timeElapsed);
	void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

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
};

class StartScene : public Scene
{
public:
	StartScene();
	~StartScene() = default;
	virtual void BuildObjects(const ComPtr<ID3D12Device>& device,
		const ComPtr<ID3D12GraphicsCommandList>& commandList,
		const ComPtr<ID3D12RootSignature>& rootSignature);
};