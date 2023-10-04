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
	void Initialize();
	void Draw(const Vector4& material,const Matrix4x4& wvpData);
	void Finalize();
	void CreateVertexData();
	void CreateWVPResource();
	void SetColor();
private:
	float pi = 3.1415f;
	uint32_t kSubdivision;
	uint32_t latIndex;
	uint32_t lonIndex;
	Sprite* sprite_;
	DirectXManager* directXManager_;
	Triangle* triangle_;
	MyEngine* myEngine_;
};

