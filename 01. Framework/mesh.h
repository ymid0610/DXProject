#pragma once
#include "stdafx.h"

// Index Buffer 미사용
class Mesh abstract
{
public:
	Mesh() = default;
	~Mesh() = default;
	
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void ReleaseUploadBuffer();

	BoundingBox GetLocalAABB() const { return m_localAABB; }
	BoundingOrientedBox GetLocalOBB() const { return m_localOBB; }

protected:
	void CreateBoundingBox(const void* vertices, UINT vertexCount, UINT stride);

protected:
	UINT						m_vertices;
	ComPtr<ID3D12Resource>		m_vertexBuffer;
	ComPtr<ID3D12Resource>		m_vertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView;

	BoundingBox					m_localAABB;
	BoundingOrientedBox			m_localOBB;
};

// Index Buffer 사용
class IndexMesh abstract : public Mesh
{
public:
	IndexMesh() = default;
	~IndexMesh() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
	virtual void ReleaseUploadBuffer() override;

protected:
	UINT						m_indices;
	ComPtr<ID3D12Resource>		m_indexBuffer;
	ComPtr<ID3D12Resource>		m_indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;
};

class CubeMesh : public Mesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};

public:
	CubeMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~CubeMesh() = default;
};

class CubeIndexMesh : public IndexMesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};

public:
	CubeIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~CubeIndexMesh() = default;
};

class PlaneMesh : public Mesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};
public:
	PlaneMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~PlaneMesh() = default;
};

class CapsuleIndexMesh : public IndexMesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};
public:
	CapsuleIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, float radius = 0.5f, float height = 1.0f, int segments = 16);
	~CapsuleIndexMesh() = default;
};
class FirstPersonGunMesh : public Mesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};

public:
	FirstPersonGunMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~FirstPersonGunMesh() = default;
};

class CrosshairMesh : public Mesh
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 colors;
	};

public:
	CrosshairMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~CrosshairMesh() = default;
};
