#include "DirectXManager.h"
#pragma once

struct TriangleData {
	Vector4 v1;
	Vector4 v2;
	Vector4 v3;
};

class Triangle
{
public:
	void Initialize(DirectXManager* directX, TriangleData left, TriangleData top, TriangleData right, TriangleData material);
	void Draw();
	void CreateVertexResource();
	void Finalize();
	void SetColor();

public:
	DirectXManager* directXManager_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	TriangleData* vertexData_;
	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	ID3D12Resource* MaterialResource_;
	TriangleData* materialData_;
};

