#include "mesh.h"

void Mesh::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->DrawInstanced(m_vertices, 1, 0, 0);
}

void Mesh::ReleaseUploadBuffer()
{
	if (m_vertexUploadBuffer) m_vertexUploadBuffer.Reset();
}

void Mesh::CreateBoundingBox(const void* vertices, UINT vertexCount, UINT stride)
{
	if (vertexCount == 0 || !vertices) return;

	XMFLOAT3 vMin{ +FLT_MAX, +FLT_MAX, +FLT_MAX };
	XMFLOAT3 vMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	// 바이트 포인터로 캐스팅해서 stride 크기만큼씩 점프하며 접근
	const BYTE* pPtr = static_cast<const BYTE*>(vertices);

	for (UINT i = 0; i < vertexCount; ++i)
	{
		// 맨 앞에 position이 있다고 가정하고 추출
		const XMFLOAT3* pPosition = reinterpret_cast<const XMFLOAT3*>(pPtr);

		vMin.x = min(vMin.x, pPosition->x);
		vMin.y = min(vMin.y, pPosition->y);
		vMin.z = min(vMin.z, pPosition->z);

		vMax.x = max(vMax.x, pPosition->x);
		vMax.y = max(vMax.y, pPosition->y);
		vMax.z = max(vMax.z, pPosition->z);

		pPtr += stride;
	}

	BoundingBox::CreateFromPoints(m_localAABB, XMLoadFloat3(&vMin), XMLoadFloat3(&vMax));

	m_localOBB.Center = m_localAABB.Center;
	m_localOBB.Extents = m_localAABB.Extents;
	m_localOBB.Orientation = XMFLOAT4{ 0.0f, 0.0f, 0.0f, 1.0f };
}

void IndexMesh::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->DrawIndexedInstanced(m_indices, 1, 0, 0, 0);
}

void IndexMesh::ReleaseUploadBuffer()
{
	Mesh::ReleaseUploadBuffer();
	if (m_indexUploadBuffer) m_indexUploadBuffer.Reset();
}

CubeMesh::CubeMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	vector<Vertex> vertices;
	const XMFLOAT3 LEFTDOWNFRONT =	{ -1.f, -1.f, -1.f };
	const XMFLOAT3 LEFTDOWNBACK =	{ -1.f, -1.f, +1.f };
	const XMFLOAT3 LEFTUPFRONT =	{ -1.f, +1.f, -1.f };
	const XMFLOAT3 LEFTUPBACK =		{ -1.f, +1.f, +1.f };
	const XMFLOAT3 RIGHTDOWNFRONT = { +1.f, -1.f, -1.f };
	const XMFLOAT3 RIGHTDOWNBACK =	{ +1.f, -1.f, +1.f };
	const XMFLOAT3 RIGHTUPFRONT =	{ +1.f, +1.f, -1.f };
	const XMFLOAT3 RIGHTUPBACK =	{ +1.f, +1.f, +1.f };

	// Front
	vertices.emplace_back(LEFTUPFRONT,		XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTUPFRONT,		XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });
	
	vertices.emplace_back(LEFTUPFRONT,		XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	
	// Up
	vertices.emplace_back(LEFTUPBACK,		XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTUPBACK,		XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });
	vertices.emplace_back(RIGHTUPFRONT,		XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	
	vertices.emplace_back(LEFTUPBACK,		XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTUPFRONT,		XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(LEFTUPFRONT,		XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });
	
	// Back
	vertices.emplace_back(LEFTDOWNBACK,		XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK,	XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTUPBACK,		XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });
	
	vertices.emplace_back(LEFTDOWNBACK,		XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTUPBACK,		XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });
	vertices.emplace_back(LEFTUPBACK,		XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	
	// Down
	vertices.emplace_back(LEFTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK,	XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	
	vertices.emplace_back(LEFTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK,	XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNBACK,		XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	
	// Left
	vertices.emplace_back(LEFTUPBACK,		XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(LEFTUPFRONT,		XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	
	vertices.emplace_back(LEFTUPBACK,		XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNBACK,		XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	
	// Right
	vertices.emplace_back(RIGHTUPFRONT,		XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTUPBACK,		XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK,	XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	
	vertices.emplace_back(RIGHTUPFRONT,		XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK,	XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNFRONT,	XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });

	m_vertices = static_cast<UINT>(vertices.size());
	const UINT vertexBufferSize = m_vertices * sizeof(Vertex);

	CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), 
		m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
}

CubeIndexMesh::CubeIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	vector<Vertex> vertices;

	const XMFLOAT3 LEFTDOWNFRONT = { -1.f, -1.f, -1.f };
	const XMFLOAT3 LEFTDOWNBACK = { -1.f, -1.f, +1.f };
	const XMFLOAT3 LEFTUPFRONT = { -1.f, +1.f, -1.f };
	const XMFLOAT3 LEFTUPBACK = { -1.f, +1.f, +1.f };
	const XMFLOAT3 RIGHTDOWNFRONT = { +1.f, -1.f, -1.f };
	const XMFLOAT3 RIGHTDOWNBACK = { +1.f, -1.f, +1.f };
	const XMFLOAT3 RIGHTUPFRONT = { +1.f, +1.f, -1.f };
	const XMFLOAT3 RIGHTUPBACK = { +1.f, +1.f, +1.f };

	vertices.emplace_back(LEFTUPBACK, XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTUPBACK, XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });
	vertices.emplace_back(RIGHTUPFRONT, XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(LEFTUPFRONT, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	vertices.emplace_back(LEFTDOWNBACK, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNBACK, XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(RIGHTDOWNFRONT, XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(LEFTDOWNFRONT, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });

	vector<UINT> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	indices.push_back(0); indices.push_back(2); indices.push_back(3);

	indices.push_back(3); indices.push_back(2); indices.push_back(6);
	indices.push_back(3); indices.push_back(6); indices.push_back(7);

	indices.push_back(7); indices.push_back(6); indices.push_back(5);
	indices.push_back(7); indices.push_back(5); indices.push_back(4);

	indices.push_back(1); indices.push_back(0); indices.push_back(4);
	indices.push_back(1); indices.push_back(4); indices.push_back(5);

	indices.push_back(0); indices.push_back(3); indices.push_back(7);
	indices.push_back(0); indices.push_back(7); indices.push_back(4);

	indices.push_back(2); indices.push_back(1); indices.push_back(5);
	indices.push_back(2); indices.push_back(5); indices.push_back(6);

	m_vertices = static_cast<UINT>(vertices.size());
	const UINT vertexBufferSize = m_vertices * sizeof(Vertex);

	CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_vertexBuffer)));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_vertexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	// 정점 버퍼 뷰 설정
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	m_indices = static_cast<UINT>(indices.size());
	const UINT indexBufferSize = m_indices * sizeof(UINT);

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		NULL,
		IID_PPV_ARGS(&m_indexBuffer)));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_indexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA indexData{};
	indexData.pData = indices.data();
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), m_indexBuffer.Get(), m_indexUploadBuffer.Get(), 0, 0, 1, &indexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = indexBufferSize;
}

PlaneMesh::PlaneMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	vector<Vertex> vertices;

	float w = 15.0f;
	float d = 15.0f;
	float y = 0.0f;

	XMFLOAT3 leftTop = { -w, y, +d };
	XMFLOAT3 rightTop = { +w, y, +d };
	XMFLOAT3 leftBottom = { -w, y, -d };
	XMFLOAT3 rightBottom = { +w, y, -d };

	XMFLOAT4 planeColor = { 0.4f, 0.6f, 0.4f, 1.0f };

	vertices.emplace_back(leftTop, planeColor);
	vertices.emplace_back(rightTop, planeColor);
	vertices.emplace_back(rightBottom, planeColor);

	vertices.emplace_back(leftTop, planeColor);
	vertices.emplace_back(rightBottom, planeColor);
	vertices.emplace_back(leftBottom, planeColor);

	m_vertices = static_cast<UINT>(vertices.size());
	const UINT vertexBufferSize = m_vertices * sizeof(Vertex);

	CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	Utiles::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(),
		m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
}

CapsuleIndexMesh::CapsuleIndexMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, float radius, float height, int segments)
{
	std::vector<Vertex> vertices;
	std::vector<UINT> indices;

	int rings = segments / 2;
	float halfHeight = height * 0.5f;
	XMFLOAT4 color = { 0.8f, 0.2f, 0.2f, 1.0f }; // 빨간색 캡슐

	// 1. 위쪽 반구 (Top Hemisphere)
	for (int i = 0; i <= rings; ++i) {
		float theta = XM_PIDIV2 - (static_cast<float>(i) / rings) * XM_PIDIV2;
		for (int j = 0; j <= segments; ++j) {
			float phi = (static_cast<float>(j) / segments) * XM_2PI;
			XMFLOAT3 pos(radius * cos(theta) * cos(phi), radius * sin(theta) + halfHeight, radius * cos(theta) * sin(phi));
			vertices.push_back({ pos, color });
		}
	}
	// 2. 원통 (Cylinder)
	// (위, 아래 경계는 반구의 끝부분과 일치하도록 이미 만들어짐)

	// 3. 아래쪽 반구 (Bottom Hemisphere)
	for (int i = 0; i <= rings; ++i) {
		float theta = -(static_cast<float>(i) / rings) * XM_PIDIV2;
		for (int j = 0; j <= segments; ++j) {
			float phi = (static_cast<float>(j) / segments) * XM_2PI;
			XMFLOAT3 pos(radius * cos(theta) * cos(phi), radius * sin(theta) - halfHeight, radius * cos(theta) * sin(phi));
			vertices.push_back({ pos, color });
		}
	}

	// 인덱스 생성
	int ringVertexCount = segments + 1;
	for (int i = 0; i < (rings * 2 + 1); ++i) {
		for (int j = 0; j < segments; ++j) {
			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j);
			indices.push_back(i * ringVertexCount + j + 1);

			indices.push_back(i * ringVertexCount + j + 1);
			indices.push_back((i + 1) * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);
		}
	}

	m_vertices = static_cast<UINT>(vertices.size());
	m_indices = static_cast<UINT>(indices.size());
	CreateBoundingBox(vertices.data(), m_vertices, sizeof(Vertex));

	const UINT vertexBufferSize = m_vertices * sizeof(Vertex);
	const UINT indexBufferSize = m_indices * sizeof(UINT);

	// Vertex Buffer
	Utiles::ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_vertexBuffer)));
	Utiles::ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexUploadBuffer)));
	D3D12_SUBRESOURCE_DATA vertexData{};
	vertexData.pData = vertices.data();
	vertexData.RowPitch = vertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), m_vertexBuffer.Get(), m_vertexUploadBuffer.Get(), 0, 0, 1, &vertexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	// Index Buffer
	Utiles::ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_indexBuffer)));
	Utiles::ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_indexUploadBuffer)));
	D3D12_SUBRESOURCE_DATA indexData{};
	indexData.pData = indices.data();
	indexData.RowPitch = indexBufferSize;
	indexData.SlicePitch = indexData.RowPitch;
	UpdateSubresources<1>(commandList.Get(), m_indexBuffer.Get(), m_indexUploadBuffer.Get(), 0, 0, 1, &indexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = indexBufferSize;
}