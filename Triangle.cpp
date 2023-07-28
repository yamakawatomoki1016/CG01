#include "Triangle.h"
#include <cassert>
#include "Vector4.h"
#include "MyEngine.h"

void Triangle::Initialize(DirectXManager* directX, TriangleData left, TriangleData top, TriangleData right, TriangleData material){
	directXManager_ = directX;

	SetColor();
	CreateVertexResource();
	//左下
	vertexData_[0] = left;
	//上
	vertexData_[1] = top;
	//右上
	vertexData_[2] = right;

	*materialData_ = material;
}

void Triangle::SetColor() {
	MaterialResource_ = CreateBufferResource(directXManager_->device_, sizeof(Vector4));
	MaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}

void Triangle::Draw()
{
	directXManager_->commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);//VSVを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけばよい
	directXManager_->commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	directXManager_->commandList_->SetGraphicsRootConstantBufferView(0, MaterialResource_->GetGPUVirtualAddress());
	//描画！（DrawCall/ドローコール）。３頂点で１つのインスタンス。インスタンスについては今後
	directXManager_->commandList_->DrawInstanced(3, 1, 0, 0);
}

void Triangle::CreateVertexResource()
{
	vertexResource_ = CreateBufferResource(directXManager_->device_, sizeof(Vector4) * 3);
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(Vector4) * 3;
	//１頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vector4);
	//書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
}

void Triangle::Finalize()
{
	MaterialResource_->Release();
	vertexResource_->Release();
}

ID3D12Resource* Triangle::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeof(Vector4) * 3;//リソースのサイズ。今回はVector4を3頂点分
	//バッファの場合はこれらは１にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	HRESULT hr =
		directXManager_->
		GetDevice()->CreateCommittedResource
		(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&vertexResource_));
	assert(SUCCEEDED(hr));

	return vertexResource_;
}
