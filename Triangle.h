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
	void Initialize(DirectXManager* directX, TriangleData left, TriangleData top, TriangleData right);
	void Draw();
	void CreateVertexResource();
	void Finalize();
	Triangle();
public:
	DirectXManager* directXManager_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	TriangleData* vertexData_;
};

