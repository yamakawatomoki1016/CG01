#include "DirectXManager.h"
#include "MyMath.h"
#include "struct.h"
#pragma once
class MyEngine;

class Triangle
{
public:
	void Initialize(DirectXManager* directX, const TriangleData& vertex, MyEngine* myengine);
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
	ID3D12Resource* MaterialResource_;
	Vector4* materialData_;
	ID3D12Resource* wvpResource_;
	Matrix4x4* wvpData_;
private:
	MyEngine* myengine_;
};

