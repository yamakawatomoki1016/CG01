#include "Sphere.h"

void Sphere::Initialize(DirectXManager* directXManager, MyEngine* myEngine)
{
	directXManager_ = directXManager;
	myEngine_ = myEngine;
	CreateVertexData();
	SetColor();
	CreateWVPResource();
}

void Sphere::Draw(const Vector4& material, const Matrix4x4& wvpData)
{
	//経度分割1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision);
	//井戸の方向に分割
	for (latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		//緯度の方向に分割しながら線を描く
		for (lonIndex = 0; lonIndex < kSubdivision; lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			//頂点データを入力する。基準点a
			vertexData_[start].position.x = cos(lat) * cos(lon);
			vertexData_[start].position.y = sin(lat);
			vertexData_[start].position.z = cos(lat) * sin(lon);
			vertexData_[start].position.w = 1.0f;
			vertexData_[start].texcoord = { 0.0f,1.0f };

			vertexData_[start + 1].position.x = cos(lat) * cos(lon);
			vertexData_[start + 1].position.y = sin(lat);
			vertexData_[start + 1].position.z = cos(lat) * sin(lon);
			vertexData_[start + 1].position.w = 1.0f;
			vertexData_[start + 1].texcoord = { 0.0f,1.0f };

			vertexData_[start + 2].position.x = cos(lat) * cos(lon);
			vertexData_[start + 2].position.y = sin(lat);
			vertexData_[start + 2].position.z = cos(lat) * sin(lon);
			vertexData_[start + 2].position.w = 1.0f;
			vertexData_[start + 2].texcoord = { 0.0f,1.0f };

			vertexData_[start + 3].position.x = cos(lat) * cos(lon);
			vertexData_[start + 3].position.y = sin(lat);
			vertexData_[start + 3].position.z = cos(lat) * sin(lon);
			vertexData_[start + 3].position.w = 1.0f;
			vertexData_[start + 3].texcoord = { 0.0f,1.0f };

			vertexData_[start + 4].position.x = cos(lat) * cos(lon);
			vertexData_[start + 4].position.y = sin(lat);
			vertexData_[start + 4].position.z = cos(lat) * sin(lon);
			vertexData_[start + 4].position.w = 1.0f;
			vertexData_[start + 4].texcoord = { 0.0f,1.0f };

			vertexData_[start + 5].position.x = cos(lat) * cos(lon);
			vertexData_[start + 5].position.y = sin(lat);
			vertexData_[start + 5].position.z = cos(lat) * sin(lon);
			vertexData_[start + 5].position.w = 1.0f;
			vertexData_[start + 5].texcoord = { 0.0f,1.0f };
			
			*materialData_ = material;
			*wvpData_ = wvpData;
			directXManager_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
			//形状を設定。PS0にせっていしているものとはまた別。同じものを設定すると考えておけばいい
			directXManager_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());//material用のCBufferの場所を設定
			directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
			directXManager_->GetCommandList()->SetGraphicsRootDescriptorTable(2, myEngine_->textureSrvHandleGPU_);
			//描画！(DrawCall/ドローコール)・3頂点で1つのインスタンス。インスタンスについては今後
			directXManager_->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		}
	}
}

void Sphere::Finalize()
{
	vertexResource_->Release();
	wvpResource_->Release();
	materialResource_->Release();
}

void Sphere::CreateVertexData()
{
	vertexResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(VertexData) * 6);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
}

void Sphere::CreateWVPResource()
{
	wvpResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	*wvpData_ = MakeIdentity4x4();
}

void Sphere::SetColor()
{
	materialResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}
