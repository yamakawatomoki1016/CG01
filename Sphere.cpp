#include "Sphere.h"

void Sphere::Initialize()
{
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
			sprite_->vertexDataSprite_[start].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start].position.y = sin(lat);
			sprite_->vertexDataSprite_[start].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start].position.w = 1.0f;
			sprite_->vertexDataSprite_[start].texcoord = { 0.0f,1.0f };

			sprite_->vertexDataSprite_[start + 1].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start + 1].position.y = sin(lat);
			sprite_->vertexDataSprite_[start + 1].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start + 1].position.w = 1.0f;
			sprite_->vertexDataSprite_[start + 1].texcoord = { 0.2f,0.8f };

			sprite_->vertexDataSprite_[start + 2].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start + 2].position.y = sin(lat);
			sprite_->vertexDataSprite_[start + 2].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start + 2].position.w = 1.0f;
			sprite_->vertexDataSprite_[start + 2].texcoord = { 0.4f,0.6f };
			
			sprite_->vertexDataSprite_[start + 3].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start + 3].position.y = sin(lat);
			sprite_->vertexDataSprite_[start + 3].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start + 3].position.w = 1.0f;
			sprite_->vertexDataSprite_[start + 3].texcoord = { 0.6f,0.4f };

			sprite_->vertexDataSprite_[start + 4].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start + 4].position.y = sin(lat);
			sprite_->vertexDataSprite_[start + 4].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start + 4].position.w = 1.0f;
			sprite_->vertexDataSprite_[start + 4].texcoord = { 0.8f,0.2f };

			sprite_->vertexDataSprite_[start + 5].position.x = cos(lat) * cos(lon);
			sprite_->vertexDataSprite_[start + 5].position.y = sin(lat);
			sprite_->vertexDataSprite_[start + 5].position.z = cos(lat) * sin(lon);
			sprite_->vertexDataSprite_[start + 5].position.w = 1.0f;
			sprite_->vertexDataSprite_[start + 5].texcoord = { 1.0f,0.0f };

			*triangle_->materialData_ = material;
			*triangle_->wvpData_ = wvpData;
			directXManager_->GetCommandList()->IASetVertexBuffers(0, 1, &triangle_->vertexBufferView_);//VBVを設定
			//形状を設定。PS0にせっていしているものとはまた別。同じものを設定すると考えておけばいい
			directXManager_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(0, triangle_->MaterialResource_->GetGPUVirtualAddress());//material用のCBufferの場所を設定
			directXManager_->GetCommandList()->SetGraphicsRootConstantBufferView(1, triangle_->wvpResource_->GetGPUVirtualAddress());
			directXManager_->GetCommandList()->SetGraphicsRootDescriptorTable(2, myEngine_->textureSrvHandleGPU_);
			//描画！(DrawCall/ドローコール)・3頂点で1つのインスタンス。インスタンスについては今後
			directXManager_->GetCommandList()->DrawInstanced(6, 1, 0, 0);
		}
	}
}

void Sphere::Finalize()
{
	triangle_->vertexResource_->Release();
	triangle_->wvpResource_->Release();
	triangle_->MaterialResource_->Release();
}

void Sphere::CreateVertexData()
{
	triangle_->vertexResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(VertexData) * 6);
	//リソースの先頭のアドレスから使う
	triangle_->vertexBufferView_.BufferLocation = triangle_->vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	triangle_->vertexBufferView_.SizeInBytes = sizeof(VertexData) * 3;
	//1頂点当たりのサイズ
	triangle_->vertexBufferView_.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	triangle_->vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&triangle_->vertexData_));
}

void Sphere::CreateWVPResource()
{
	triangle_->wvpResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Matrix4x4));
	triangle_->wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&triangle_->wvpData_));
	*triangle_->wvpData_ = MakeIdentity4x4();
}

void Sphere::SetColor()
{
	triangle_->MaterialResource_ = triangle_->CreateBufferResource(directXManager_->GetDevice(), sizeof(Vector4));
	triangle_->MaterialResource_->Map(0, nullptr, reinterpret_cast<void**>(&triangle_->materialData_));
}
