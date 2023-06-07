#include "DirectXManager.h"
#include <algorithm>
#include <cassert>
#include <thread>
#include <timeapi.h>
#include <vector>
#include <dxgidebug.h>
#include "Vector4.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Winmm.lib")

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

void DirectXManager::Initialize(WinApp* win, int32_t backBufferWidth, int32_t backBufferHeight)
{
	winApp_ = win;
	backBufferWidth_ = backBufferWidth;
	backBufferHeight_ = backBufferHeight;
	winApp_->CreateGameWindow(L"CG2", 1280, 720);
	// DXGIデバイス初期化
	InitializeDXGIDevice();

	// コマンド関連初期化
	InitializeCommand();

	// スワップチェーンの生成
	CreateSwapChain();

	// レンダーターゲット生成
	CreateFinalRenderTargets();

	// フェンス生成
	CreateFence();
}

IDxcBlob* DirectXManager::CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler)
{
	//これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr_ = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource_);
	//読めなかったら止める
	assert(SUCCEEDED(hr_));
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource_->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource_->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTFの文字コードであることを通知
	LPCWSTR arguments[] = {
		filePath.c_str(), //コンパイル対象hlslファイル名
		L"-E",L"main",   //エントリーポイントの指定。基本的にmain以外にはしない
		L"-T",profile,   //ShaderProfileの設定
		L"-Zi",L"-Qembed_debug",//デバッグ用の情報を埋め込む
		L"-0d",  //最適化を外しておく
		L"-Zpr",  //メモリレイアウトは行優先
	};

	hr_ = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult_)
	);
	//コンパイルエラーではなくdxが起動できないなど致命的な状況
	assert(SUCCEEDED(hr_));

	//警告・エラーが出てたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult_->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		//警告・ダメゼッタイ
		assert(false);
	}

	//コンパイル結果から実行用のπなり部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr_ = shaderResult_->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr_));
	//成功したらログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));
	
	return nullptr;
	//実行用のパイナリを返却
	return shaderBlob;

}

//デバイスの作成
void DirectXManager::InitializeDXGIDevice() {
	dxgiFactory_ = nullptr;
	hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	assert(SUCCEEDED(hr_));
	useAdapter_ = nullptr;
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr_ = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr_));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr;
	}
	
	assert(useAdapter_ != nullptr);
	device_ = nullptr;
	D3D_FEATURE_LEVEL featurelevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featurelevels); ++i) {
		hr_ = D3D12CreateDevice(useAdapter_, featurelevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr_)) {
			Log(std::format("Featurelevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!!\n");
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//ヤバいエラーで止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//抑制するメッセージのID 
		D3D12_MESSAGE_ID denyIds[] = {
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		//解放
		infoQueue->Release();
	}
#endif
}
////コマンド関連の初期化
void DirectXManager::InitializeCommand() {
	//コマンドキューを生成する
	commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr_ = device_->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));

	//コマンドアロケータを生成する
	commandAllocator_ = nullptr;
	hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator_));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));
	//コマンドリストを生成する
	commandList_ = nullptr;
	hr_ = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr,
		IID_PPV_ARGS(&commandList_));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr_));
}
//スワップチェーンを生成
void DirectXManager::CreateSwapChain() {
	swapChain_ = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = winApp_->GetWidth();//画面の幅
	swapChainDesc.Height = winApp_->GetHeight();//画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタにうつしたら中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr_ = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_, winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr_));

	//ディスクリプタヒープの生成
	rtvDescriptorHeap_ = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptionHeapDesc{};
	rtvDescriptionHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビュー用
	rtvDescriptionHeapDesc.NumDescriptors = 2;//ダブルバッファ用に二つ。多くても別にかまわない
	hr_ = device_->CreateDescriptorHeap(&rtvDescriptionHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap_));
	assert(SUCCEEDED(hr_));
	//SwapChainからResourceを引っ張ってくる
	backBuffers_[0] = { nullptr };
	backBuffers_[1] = { nullptr };
	hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffers_[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr_));
	hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&backBuffers_[1]));
	assert(SUCCEEDED(hr_));
}

// レンダーターゲット生成
void DirectXManager::CreateFinalRenderTargets() {

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2Dテクスチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	//まず一つ目を作る。一つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(backBuffers_[0], &rtvDesc, rtvHandles_[0]);
	//2つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device_->CreateRenderTargetView(backBuffers_[1], &rtvDesc, rtvHandles_[1]);

}
// フェンス生成
void DirectXManager::CreateFence() {
	//初期値０でFenceを作る
	fence_ = nullptr;
	fenceVal_ = 0;
	hr_ = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr_));
	//fenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);

}

void DirectXManager::IntializeDxcCompiler()
{
	//dxcCompilerを初期化
	hr_ = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr_));
	hr_ = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr_));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr_ = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr_));

}

void DirectXManager::CreateRootSignature()
{
	//RootSignature作成
	descriptionRootSignature_.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//シリアライズしてパイナリにする
	hr_ = D3D12SerializeRootSignature(&descriptionRootSignature_,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr_)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	//パイナリを元に生成
	hr_ = device_->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr_));
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);
}

void DirectXManager::BlendSetting()
{
	//すべての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
}

void DirectXManager::RasiterzerState()
{
	//裏面（時計回り）を表示しない
	rasterzerDesc_.CullMode = D3D12_CULL_MODE_BACK;
	//三角形を塗りつぶす
	rasterzerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
}

void DirectXManager::CreateGraphicsPipelineState()
{
	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3D.VS.hlsl",
		L"vs_6_0", dxcUtils_, dxcCompiler_, includeHandler_);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3D.PS.hlsl",
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
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr_ = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr_));
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
}

void DirectXManager::CreateVertexBufferView()
{
	//リソースの戦闘のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(Vector4) * 3;
	//１頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(Vector4);
}

void DirectXManager::VertexDate()
{
	Vector4* vertexDate = nullptr;
	//書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));
	//左下
	vertexDate[0] = { -0.5f,-0.5f,0.0f,1.0f };
	//上
	vertexDate[1] = { 0.0f,0.5f,0.0f,1.0f };
	//右下
	vertexDate[2] = { 0.5f,-0.5f,0.0f,1.0f };
}

void DirectXManager::Viewport()
{
	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = backBufferWidth_;
	viewport_.Height = backBufferHeight_;
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
	
	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = backBufferHeight_;
	scissorRect_.top = 0;
	scissorRect_.bottom = backBufferWidth_;
}

void DirectXManager::CreateVertexResource()
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
	hr_ = device_->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_DEPTH_READ, nullptr,
		IID_PPV_ARGS(&vertexResource_));
	assert(SUCCEEDED(hr_));
}

void DirectXManager::PreDraw()
{
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier_.Transition.pResource = backBuffers_[backBufferIndex];
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrier_);
	DirectXManager::ClearRenderTarget();
}

void DirectXManager::PostDraw() {
	barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitonBarrierを張る
	commandList_->ResourceBarrier(1, &barrier_);

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	hr_ = commandList_->Close();
	assert(SUCCEEDED(hr_));

	//GPUにコマンドリストを準備する
	ID3D12CommandList* commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists);
	//GPUとOSに画面の交換を行うように通知する
	swapChain_->Present(1, 0);

	//Fenceの値を更新
	fenceVal_++;
	//GPUがここまでたどり着いたときに、Fenceの値を指定し値に代入するようにSignalを送る
	commandQueue_->Signal(fence_, fenceVal_);

	//Fenceの値が指定したSignal値にたどりつうてるじゃ確認する
    //GetCompletevaluseの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceVal_) {
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
		//イベント待つ
		WaitForSingleObject(fenceEvent_, INFINITE);

	}
	//次のフレーム用のコマンドリストを準備
	hr_ = commandAllocator_->Reset();
	assert(SUCCEEDED(hr_));
	hr_ = commandList_->Reset(commandAllocator_, nullptr);
	assert(SUCCEEDED(hr_));
	commandList_->RSSetViewports(1, &viewport_);//Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect_);//Scirssorを設定
	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetGraphicsRootSignature(rootSignature_);
	commandList_->SetPipelineState(graphicsPipelineState_);//PSOを設定
	commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_);//VSVを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけばよい
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//描画！（DrawCall/ドローコール）。３頂点で１つのインスタンス。インスタンスについては今後
	commandList_->DrawInstanced(3, 1, 0, 0);
}

void DirectXManager::ClearRenderTarget()
{
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);
	//指定した色で画面全体をクリアする
	float clearcolor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearcolor, 0, nullptr);
}

void DirectXManager::Finalize() {
	CloseHandle(fenceEvent_);
	fence_->Release();
	rtvDescriptorHeap_->Release();
	backBuffers_[0]->Release();
	backBuffers_[1]->Release();
	swapChain_->Release();
	commandList_->Release();
	commandAllocator_->Release();
	commandQueue_->Release();
	device_->Release();
	useAdapter_->Release();
	dxgiFactory_->Release();
	shaderSource_->Release();
	shaderResult_->Release();
	vertexResource_->Release();
	graphicsPipelineState_->Release();
	signatureBlob_->Release();
	if (errorBlob_) {
		errorBlob_->Release();
	}
	rootSignature_->Release();
#ifdef DEBUG
	winApp_->GetdebugController()->Release();
#endif // DEBUG

	CloseWindow(winApp_->GetHwnd());
	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
}
WinApp* DirectXManager::winApp_;
IDXGIAdapter4* DirectXManager::useAdapter_;
IDXGIFactory7* DirectXManager::dxgiFactory_;
ID3D12Device* DirectXManager::device_;
ID3D12CommandQueue* DirectXManager::commandQueue_;
ID3D12CommandAllocator* DirectXManager::commandAllocator_;
ID3D12GraphicsCommandList* DirectXManager::commandList_;
IDXGISwapChain4* DirectXManager::swapChain_;
ID3D12DescriptorHeap* DirectXManager::rtvDescriptorHeap_;
D3D12_CPU_DESCRIPTOR_HANDLE DirectXManager::rtvHandles_[2];
ID3D12Resource* DirectXManager::backBuffers_[2];
UINT64 DirectXManager::fenceVal_;
int32_t DirectXManager::backBufferWidth_;
int32_t DirectXManager::backBufferHeight_;
ID3D12Fence* DirectXManager::fence_;
HANDLE DirectXManager::fenceEvent_;
HRESULT DirectXManager::hr_;