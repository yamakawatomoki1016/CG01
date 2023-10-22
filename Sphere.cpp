#include "Sphere.h"

void Sphere::Initialize(DirectXManager* directXManager, MyEngine* myEngine)
{
	
	directXManager_ = directXManager;
	myEngine_ = myEngine;
	kSubdivision = 16;
	vertexCount_ = kSubdivision * kSubdivision * 6;
	CreateVertexData();
	SetColor();
	CreateWVPResource();
	//経度分割1つ分の角度
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割1つ分の角度
	const float kLatEvery = pi / float(kSubdivision);
	for (latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;
		//緯度の方向に分割しながら線を描く
		for (lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;
			//頂点データを入力する。基準点a
			vertexData_[start].position = { cos(lat) * cos(lon),sin(lat),cos(lat) * sin(lon),1.0f };
			vertexData_[start].texcoord = { float(lonIndex) / float(kSubdivision),1.0f - float(latIndex) / kSubdivision };

			vertexData_[start + 1].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon),1.0f };
			vertexData_[start + 1].texcoord = { vertexData_[start].texcoord.x,vertexData_[start].texcoord.y - 1.0f / float(kSubdivision) };

			vertexData_[start + 2].position = { cos(lat) * cos(lon + kLonEvery),sin(lat),cos(lat) * sin(lon + kLonEvery),1.0f };
			vertexData_[start + 2].texcoord = { vertexData_[start].texcoord.x + 1.0f / float(kSubdivision),vertexData_[start].texcoord.y };

			vertexData_[start + 3].position = { cos(lat) * cos(lon + kLonEvery),sin(lat),cos(lat) * sin(lon + kLonEvery),1.0f };
			vertexData_[start + 3].texcoord = { vertexData_[start].texcoord.x + 1.0f / float(kSubdivision),vertexData_[start].texcoord.y };

			vertexData_[start + 4].position = { cos(lat + kLatEvery) * cos(lon),sin(lat + kLatEvery),cos(lat + kLatEvery) * sin(lon),1.0f };
			vertexData_[start + 4].texcoord = { vertexData_[start].texcoord.x,vertexData_[start].texcoord.y - 1.0f / float(kSubdivision) };

			vertexData_[start + 5].position = { cos(lat + kLatEvery) * cos(lon + kLonEvery),sin(lat + kLatEvery), cos(lat + kLatEvery) * sin(lon + kLonEvery),1.0f };
			vertexData_[start + 5].texcoord = { vertexData_[start].texcoord.x + 1.0f / float(kSubdivision),vertexData_[start].texcoord.y - 1.0f / float(kSubdivision) };


		}
	}
}

void Sphere::Draw(const Vector4& material, const Matrix4x4& wvpData)
{
	//井戸の方向に分割
	*materialData_ = material;
	*wvpData_ = wvpData;
	directXManager_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);//VBVを設定
	//形状を設定。PS0にせっていしているものとはまた別。同じものを設定すると考えておけばいい
	directXManager_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());//material用のCBufferの場所を設定
	directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	directXManager_->GetCommandList()->SetGraphicsRootDescriptorTable(2, myEngine_->textureSrvHandleGPU_);
	//描画！(DrawCall/ドローコール)・3頂点で1つのインスタンス。インスタンスについては今後
	directXManager_->GetCommandList()->DrawInstanced(vertexCount_, 1, 0, 0);

}

void Sphere::Finalize()
{
	vertexResource_->Release();
	wvpResource_->Release();
	materialResource_->Release();
}

void Sphere::CreateVertexData()
{
	vertexResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(VertexData) * vertexCount_);
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * vertexCount_;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
}

void Sphere::CreateWVPResource()
{
	wvpResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	*wvpData_ = MakeIdentity4x4();
}

void Sphere::SetColor()
{
	materialResource_ = directXManager_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
}
