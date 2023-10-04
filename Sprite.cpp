#include "Sprite.h"

void Sprite::Initialize(DirectXManager* directXManager)
{
	directXManager_ = directXManager;
	CreateVertexData();
	CreateTransform();
}

void Sprite::Draw(Transform* transform)
{
	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transform->scale, transform->rotate, transform->translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(winApp_->GetWidth()), float(winApp_->GetHeight()), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectMatrixSprite));
	*transformationMatrixDataSprite_ = worldViewProjectionMatrixSprite;

	directXManager_->commandList_->IASetVertexBuffers(0, 1, &vertexBufferViewSprite_);
	directXManager_->commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite_->GetGPUVirtualAddress());
	directXManager_->commandList_->DrawInstanced(6, 1, 0, 0);
}

void Sprite::Finalize()
{
	vertexResourceSprite_->Release();
	transformationMatrixResourceSprite_->Release();
}

void Sprite::CreateVertexData()
{
	
	vertexResourceSprite_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(VertexData) * 6);
	vertexBufferViewSprite_.BufferLocation = vertexResourceSprite_->GetGPUVirtualAddress();
	vertexBufferViewSprite_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite_.StrideInBytes = sizeof(VertexData);

	vertexResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite_));
	vertexDataSprite_[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDataSprite_[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite_[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite_[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataSprite_[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDataSprite_[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDataSprite_[0].texcoord = { 0.0f,1.0f };
	vertexDataSprite_[1].texcoord = { 0.0f,0.0f };
	vertexDataSprite_[2].texcoord = { 1.0f,1.0f };
	vertexDataSprite_[3].texcoord = { 0.0f,0.0f };
	vertexDataSprite_[4].texcoord = { 1.0f,0.0f };
	vertexDataSprite_[5].texcoord = { 1.0f,1.0f };

}

void Sprite::CreateTransform()
{
	transformationMatrixResourceSprite_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
	transformationMatrixResourceSprite_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite_));
	*transformationMatrixDataSprite_ = MakeIdentity4x4();
}
