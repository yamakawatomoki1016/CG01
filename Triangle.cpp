#include "Triangle.h"
#include <assert.h>


void Triangle::Initialize(DirectXManager* directX, const TriangleData& vertex)
{
	directXManager_ = directX;
	CreateVertexResource();
	SetColor();
	CreateWVPResource();
	//左下
	vertexData_->v1 = vertex.v1;
	//上
	vertexData_->v2 = vertex.v2;
	//右下
	vertexData_->v3 = vertex.v3;
}
void Triangle::SetColor() {
	MaterialResource_ = CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4));
	MaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}
void Triangle::CreateWVPResource()
{
	wvpResource_ = CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
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


	vertexResource_ = CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4) * 3);
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(Vector4) * 3;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vector4);
	//書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

}

ID3D12Resource* Triangle::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
	//頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uplodeHeapProperties{};
	uplodeHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//頂点リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;//リソースサイズ
	//バッファの場合はこれらは１にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	HRESULT hr;
	ID3D12Resource* Resource = nullptr;
	//実際に頂点リソースを作る
	hr = device->CreateCommittedResource(&uplodeHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Resource));
	assert(SUCCEEDED(hr));

	return Resource;
}