#include "Triangle.h"
#include <assert.h>
#include "MyEngine.h"


void Triangle::Initialize(DirectXManager* directX, const TriangleData& vertex, MyEngine* myengine)
{
	directXManager_ = directX;
	myengine_ = myengine;
	CreateVertexResource();
	SetColor();
	CreateWVPResource();
	//左下
	vertexData_->v1.position = vertex.v1.position;
	vertexData_->v1.texcoord = { 0.0f,1.0f };
	//上
	vertexData_->v2.position = vertex.v2.position;
	vertexData_->v2.texcoord = { 0.5f,0.0f };
	//右下
	vertexData_->v3.position = vertex.v3.position;
	vertexData_->v3.texcoord = { 1.0f,1.0f };
}
void Triangle::SetColor() {
	MaterialResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4));
	MaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}
void Triangle::CreateWVPResource()
{
	wvpResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	*wvpData_ = MakeIdentity4x4();
}
void Triangle::Draw(const Vector4& material, const Matrix4x4& worldMatrix)
{
	*wvpData_ = worldMatrix;
	*materialData_ = material;
	directXManager_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
	//形状を設定。PS0にせっていしているものとはまた別。同じものを設定すると考えておけばいい
	directXManager_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(0, MaterialResource_->GetGPUVirtualAddress());//material用のCBufferの場所を設定
	directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	directXManager_->GetCommandList()->SetGraphicsRootDescriptorTable(2, myengine_->textureSrvHandleGPU_);
	//描画！(DrawCall/ドローコール)・3頂点で1つのインスタンス。インスタンスについては今後
	directXManager_->GetCommandList()->DrawInstanced(3, 1, 0, 0);

}
void Triangle::Finalize()
{
	wvpResource_->Release();
	MaterialResource_->Release();
	vertexResource_->Release();
}
void Triangle::CreateVertexResource() {


	vertexResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(VertexData) * 3);
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

}