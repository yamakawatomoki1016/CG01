#include "MyEngine.h"
#include <assert.h>
void MyEngine::Initialize(WinApp* win, int32_t backBufferWidth, int32_t backBufferHeight)
{
	directXManager = new DirectXManager();
	directXManager->Initialize(win,backBufferWidth,backBufferHeight);

	IntializeDxcCompiler();

	CreateRootSignature();

	BlendSetting();

	RasiterzerState();

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
	for (int i = 0; i < 10; i++) {
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
	//シリアライズしてパイナリにする
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	//パイナリを元に生成
	hr =directXManager->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
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
	//実際に生成
	HRESULT hr = directXManager->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));

}

void MyEngine::VariableInitialize()
{
	for (int i = 0; i < 10; i++) {
		triangle_[i] = new Triangle();
		triangle_[i]->Initialize(directXManager);
	}
	left_[0] = { -0.2f,-0.1f,0.0f,1.0f };
	top_[0] = { -0.15f,0.1f,0.0f,1.0f };
	right_[0] = { -0.1f,-0.1f,0.0f,1.0f };

	left_[1] = { -0.2f,-0.3f,0.0f,1.0f };
	top_[1] = { -0.15f,-0.1f,0.0f,1.0f };
	right_[1] = { -0.1f,-0.3f,0.0f,1.0f };

	left_[2] = { -0.2f,-0.5f,0.0f,1.0f };
	top_[2] = { -0.15f,-0.3f,0.0f,1.0f };
	right_[2] = { -0.1f,-0.5f,0.0f,1.0f };

	left_[3] = { -0.2f,-0.7f,0.0f,1.0f };
	top_[3] = { -0.15f,-0.5f,0.0f,1.0f };
	right_[3] = { -0.1f,-0.7f,0.0f,1.0f };

	left_[4] = { -0.2f,-0.9f,0.0f,1.0f };
	top_[4] = { -0.15f,-0.7f,0.0f,1.0f };
	right_[4] = { -0.1f,-0.9f,0.0f,1.0f };

	left_[5] = { -0.2f,0.1f,0.0f,1.0f };
	top_[5] = { -0.15f,0.3f,0.0f,1.0f };
	right_[5] = { -0.1f,0.1f,0.0f,1.0f };

	left_[6] = { -0.2f,0.3f,0.0f,1.0f };
	top_[6] = { -0.15f,0.5f,0.0f,1.0f };
	right_[6] = { -0.1f,0.3f,0.0f,1.0f };

	left_[7] = { -0.2f,0.5f,0.0f,1.0f };
	top_[7] = { -0.15f,0.7f,0.0f,1.0f };
	right_[7] = { -0.1f,0.5f,0.0f,1.0f };

	left_[8] = { -0.2f,0.7f,0.0f,1.0f };
	top_[8] = { -0.15f,0.9f,0.0f,1.0f };
	right_[8] = { -0.1f,0.7f,0.0f,1.0f };

	left_[9] = { -0.1f,0.1f,0.0f,1.0f };
	top_[9] = { 0.25f,0.3f,0.0f,1.0f };
	right_[9] = { 0.6f,0.1f,0.0f,1.0f };
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

void MyEngine::Draw()
{
	for (int i = 0; i < 10; i++) {
		triangle_[i]->Draw(left_[i],top_[i],right_[i]);
	}
}

DirectXManager* MyEngine::directXManager;
WinApp* MyEngine::winApp_;