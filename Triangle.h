#include "DirectXManager.h"
#include "MyMath.h"
#pragma once

struct TriangleData {
	Vector4 v1;
	Vector4 v2;
	Vector4 v3;
};

class Triangle
{
public:
	void Initialize(DirectXManager* directX, const TriangleData& vertex);
	void Draw(const Vector4& material,const Matrix4x4& worldMatrix);
	void CreateVertexResource();
	void Finalize();
	void SetColor();
	void CreateWVPResource();
public:
	DirectXManager* directXManager_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	TriangleData* vertexData_;
	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	ID3D12Resource* MaterialResource_;
	Vector4* materialData_;
	ID3D12Resource* wvpResource_;
	Matrix4x4* wvpData_;
};

