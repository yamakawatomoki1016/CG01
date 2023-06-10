#include "DirectXManager.h"


#pragma once

class Triangle
{
public:
	void Initialize(DirectXManager* directX);
	void Draw(Vector4 left, Vector4 top, Vector4 right);
	void CreateVertexResource();
	void Finalize();
	Triangle();
public:
	DirectXManager* directXManager_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Vector4* vertexDate_;
};

