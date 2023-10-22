#include "MyEngine.h"
#include <assert.h>
#include "Convert.h"
void MyEngine::Initialize(WinApp* win, int32_t backBufferWidth, int32_t backBufferHeight)
{
	directXManager = new DirectXManager();
	directXManager->Initialize(win,backBufferWidth,backBufferHeight);

	IntializeDxcCompiler();
	CreateRootSignature();
	BlendSetting();
	RasiterzerState();
	SetDepth();
	CreateGraphicsPipelineState();
	Viewport();
}

IDxcBlob* MyEngine::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler)
{
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr_ = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr_));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTFの文字コードであることを通知
	LPCWSTR arguments[] = {
		filePath.c_str(), //コンパイル対象hlslファイル名
		L"-E",L"main",   //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,   //ShaderProfileの設定
		L"-Zi",L"-Qembed_debug",//デバッグ用の情報を埋め込む
		L"-Od",  //最適化を外しておく
		L"-Zpr",  //メモリレイアウトは行優先
	};
	IDxcResult* shaderResult = nullptr;
	hr_ = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult)
	);
	//コンパイルエラーではなくdxが起動できないなど致命的な状況
	assert(SUCCEEDED(hr_));

	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		//警告・ダメゼッタイ
		assert(false);
	}

	//コンパイル結果から実行用のπなり部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr_ = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));
	//成功したらログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	shaderSource->Release();
	shaderResult->Release();
	//実行用のパイナリを返却
	return shaderBlob;

}

void MyEngine::BeginFrame()
{
	directXManager->PreDraw();
	directXManager->GetCommandList()->RSSetViewports(1, &viewport_);//Viewportを設定
	directXManager->GetCommandList()->RSSetScissorRects(1, &scissorRect_);//Scirssorを設定
	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	directXManager->GetCommandList()->SetGraphicsRootSignature(rootSignature_);
	directXManager->GetCommandList()->SetPipelineState(graphicsPipelineState_);//PSOを設定
}

void MyEngine::EndFrame()
{
	directXManager->PostDraw();
}

void MyEngine::Finalize()
{
	sprite_->Finalize();
	textureResource_->Release();
	for (int i = 0; i < 2; i++) {
		triangle_[i]->Finalize();
	}
	graphicsPipelineState_->Release();
	signatureBlob_->Release();
	if (errorBlob_) {
		errorBlob_->Release();
	}
	rootSignature_->Release();
	directXManager->Finalize();
}

void MyEngine::Update()
{
	ImGui::ShowDemoWindow();
	triangleTransform_.rotate.y += 0.001f;
	worldMatrix_ = MakeAffineMatrix(triangleTransform_.scale, triangleTransform_.rotate, triangleTransform_.translate);
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	Matrix4x4 viewMatrix = Inverse(cameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(directXManager->winApp_->GetWidth()) / float(directXManager->winApp_->GetHeight()), 0.1f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));
	worldMatrix_ = worldViewProjectionMatrix;
	ImGui::Begin("Triangle");
	ImGui::DragFloat3("Translate",&triangleTransform_.rotate.x,0.1f);
	ImGui::DragFloat3("Translate", &transformSprite_.translate.x, 0.1f);
	ImGui::ColorEdit4("Color Picker", &material[1].x);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall);
	ImGui::End();
}

void MyEngine::IntializeDxcCompiler()
{
	//dxcCompilerを初期化
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void MyEngine::CreateRootSignature()
{
	//RootSignature作成
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ一の配列
	rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; //CBVを使う
	rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; //PixelShaderで使う
	rootParameters_[0].Descriptor.ShaderRegister = 0; //レジスタ番号0とバインド
	rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters_[1].Descriptor.ShaderRegister = 0;
	rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange_;
	rootParameters_[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange_);
	descriptorRange_[0].BaseShaderRegister = 0;
	descriptorRange_[0].NumDescriptors = 1;
	descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers_[0].ShaderRegister = 0;
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature_.pStaticSamplers = staticSamplers_;
	descriptionRootSignature_.NumStaticSamplers = _countof(staticSamplers_);
		descriptionRootSignature_.pParameters = rootParameters_; //ルートパラメータ―配列へのポインタ
	descriptionRootSignature_.NumParameters = _countof(rootParameters_); //配列の長さ
	//シリアライズしてパイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	//パイナリを元に生成
	hr = directXManager->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);
}

void MyEngine::BlendSetting()
{
	//すべての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
}

void MyEngine::RasiterzerState()
{
	//裏面（時計回り）を表示しない
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
	//三角形を塗りつぶす
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
}

void MyEngine::CreateGraphicsPipelineState()
{
	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(pixelShaderBlob != nullptr);

	graphicsPipelineStateDesc_.pRootSignature = rootSignature_;//RootSignature
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDesc_;//InpytLayout
	graphicsPipelineStateDesc_.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPipelineStateDesc_.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPipelineStateDesc_.BlendState = blendDesc_;//BlendState
	graphicsPipelineStateDesc_.RasterizerState = rasterizerDesc_;//RasterizerState
	//書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトロポジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc_.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定（気にしなくてよい）
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc_.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//実際に生成
	HRESULT hr = directXManager->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));

}

void MyEngine::VariableInitialize()
{
	triangleVertex_[0].v1.position = { -0.5f,-0.5f,0.0f,1.0f };
	triangleVertex_[0].v2.position = { -0.0f,0.5f,0.0f,1.0f };
	triangleVertex_[0].v3.position = { 0.5f,-0.5f,0.0f,1.0f };

	triangleVertex_[1].v1.position = { -0.3f,-0.5f,0.5f,1.0f };
	triangleVertex_[1].v2.position = { -0.0f,-0.0f,0.0f,1.0f };
	triangleVertex_[1].v3.position = { 0.5f,-0.5f,-0.5f,1.0f };

	sprite_ = new Sprite();
	sprite_->Initialize(directXManager);
	/*sphere_ = new Sphere();
	sphere_->Initialize(directXManager,this);*/
	transformSprite_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	
	for (int i = 0; i < 2; i++) {
		triangle_[i] = new Triangle();
		triangle_[i]->Initialize(directXManager, triangleVertex_[i],this);
		material[i] = { 1.0f,1.0f,1.0f,1.0f };
	}
	triangleTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };
	LoadTexture("Resources/uvChecker.png");
}

void MyEngine::Viewport()
{
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = winApp_->kClientWidth;
	viewport_.Height = winApp_->kClientHeight;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = winApp_->kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = winApp_->kClientHeight;
}

DirectX::ScratchImage MyEngine::LoadTex(const std::string& filePath)
{
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	return mipImages;
}

ID3D12Resource* MyEngine::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

void MyEngine::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Resource* texture2, const DirectX::ScratchImage& mipImages2)
{
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages2.GetImage(mipLevel, 0, 0);
		HRESULT hr = texture2->WriteToSubresource(
			UINT(mipLevel),
			nullptr,
			img->pixels,
			UINT(img->rowPitch),
			UINT(img->slicePitch)
		);
		assert(SUCCEEDED(hr));
	}
}

void MyEngine::LoadTexture(const std::string& filePath)
{
	DirectX::ScratchImage mipImages = LoadTex(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = CreateTextureResource(directXManager->GetDevice(), metadata);
	

	DirectX::ScratchImage mipImages2 = LoadTex("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	textureResource2_ = CreateTextureResource(directXManager->GetDevice(), metadata2);
	UploadTextureData(textureResource_, mipImages,textureResource2_, mipImages2);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	textureSrvHandleCPU_ = directXManager->GetCPUDescriptorHandle(directXManager->srvDescriptorHeap_, directXManager->descriptorSizeRTV_, 1);
	textureSrvHandleGPU_ = directXManager->GetGpuDescriptorHandle(directXManager->srvDescriptorHeap_, directXManager->descriptorSizeSRV_, 1);
	textureSrvHandleCPU_.ptr += directXManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU_.ptr += directXManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	directXManager->GetDevice()->CreateShaderResourceView(textureResource_, &srvDesc, textureSrvHandleCPU_);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	textureSrvHandleCPU2_ = directXManager->GetCPUDescriptorHandle(directXManager->srvDescriptorHeap_, directXManager->descriptorSizeRTV_, 2);
	textureSrvHandleGPU2_ = directXManager->GetGpuDescriptorHandle(directXManager->srvDescriptorHeap_, directXManager->descriptorSizeSRV_, 2);
	textureSrvHandleCPU2_.ptr += directXManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU2_.ptr += directXManager->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	directXManager->GetDevice()->CreateShaderResourceView(textureResource2_, &srvDesc2, textureSrvHandleCPU2_);
}

void MyEngine::SetDepth()
{
	depthStencilDesc_.DepthEnable = true;
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void MyEngine::Draw()
{
	for (int i = 0; i < 2; i++) {
		triangle_[i]->Draw(material[i],worldMatrix_);
	}
	sprite_->Draw(&transformSprite_);
	//sphere_->Draw(material[0], worldMatrix_);
}

DirectXManager* MyEngine::directXManager;
WinApp* MyEngine::winApp_;