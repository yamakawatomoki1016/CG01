#pragma once
#include "DirectXManager.h"
#include "Vector4.h"
#include "MyEngine.h"
#include "Sprite.h"
#include "Triangle.h"
#include "Matrix4x4.h"

class Sphere
{
public:
	void Initialize(DirectXManager* directXManager,MyEngine* myEngine);
	void Draw(const Vector4& material,const Matrix4x4& wvpData);
	void Finalize();
	void CreateVertexData();
	void CreateWVPResource();
	void SetColor();
private:
	float pi = 3.1415f;
	VertexData* vertexData_;
	uint32_t kSubdivision;
	ID3D12Resource* wvpResource_;
	Matrix4x4* wvpData_;
	ID3D12Resource* materialResource_;
	ID3D12Resource* vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	Vector4* materialData_;
	uint32_t latIndex;
	uint32_t lonIndex;
	DirectXManager* directXManager_;
	Triangle* triangle_;
	MyEngine* myEngine_;
};

