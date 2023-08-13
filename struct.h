#pragma once
#include "Vector4.h"
#include "Vector2.h"
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};

struct TriangleData {
	VertexData v1;
	VertexData v2;
	VertexData v3;
};