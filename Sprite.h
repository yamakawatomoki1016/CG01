#pragma once
#include "DirectXManager.h"
#include "Triangle.h"
#include "struct.h"
#include "WinApp.h"
#include "Vector4.h"
class Sprite
{
public:
	void Initialize(DirectXManager* directXManager);
	void Draw();
	void CreateVertexData();
	void CreateTransform();
	void Finalize();
private:
	ID3D12Resource* vertexResorceSprite_;
	DirectXManager* directXManager_;
	Triangle* triangle_;
	VertexData* vertexDataSprite_{};
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite_{};
	ID3D12Resource* transformationMatrixResourceSprite_;
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	WinApp* winApp_;
	Matrix4x4* transformationMatrixDataSprite_;
};

