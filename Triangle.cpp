#include "Triangle.h"
#include <cassert>
#include "Vector4.h"
#include "MyEngine.h"

void Triangle::Initialize(DirectXManager* directX, Vector4 left, Vector4 top, Vector4 right){
	directXManager_ = directX;
	//左下
	vertexDate_[0] = left;
	//上
	vertexDate_[1] = top;
	//右上
	vertexDate_[2] = right;
	CreateVertexResource();
}

void Triangle::Draw()
{
	directXManager_->commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);//VSVを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけばよい
	directXManager_->commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画！（DrawCall/ドローコール）。３頂点で１つのインスタンス。インスタンスについては今後
	directXManager_->commandList_->DrawInstanced(3, 1, 0, 0);
}

void Triangle::CreateVertexResource()
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

	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(Vector4) * 3;
	//１頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vector4);
	//書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate_));
}

void Triangle::Finalize()
{
	vertexResource_->Release();
}

Triangle::Triangle()
{
	
}
