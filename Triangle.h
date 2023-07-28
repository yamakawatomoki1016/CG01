#include "DirectXManager.h"


#pragma once

class Triangle
{
public:
	void Initialize(DirectXManager* directX, Vector4 left, Vector4 top, Vector4 right, Vector4 material);
	void Draw();
	void CreateVertexResource();
	void Finalize();
	void SetColor();
public:
	DirectXManager* directXManager_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Vector4* vertexDate_;
	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	ID3D12Resource* MaterialResource_;
	Vector4* materialData_;
};

