#pragma once
#include "WinApp.h"
#include "conbert.h"
#include <dxcapi.h>
#include "Vector4.h"
#include "Triangle.h"
#include "DirectXManager.h"
#pragma comment(lib, "dxcompiler.lib")

class MyEngine
{
public:
	void Initialize(WinApp* win, int32_t backBufferWidth, int32_t backBufferHeight);
	void VariableInitialize();
	void Draw();
	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);
	void BeginFrame();
	void EndFrame();
	void Finalize();
private:
	void IntializeDxcCompiler();
	void CreateRootSignature();
	void BlendSetting();
	void RasiterzerState();
	void CreateGraphicsPipelineState();
	void Viewport();
	static DirectXManager* directXManager;
	Triangle* triangle_[10];
	Vector4 left_[10];
	Vector4 top_[10];
	Vector4 right_[10];
	static WinApp* winApp_;
    D3D12_INPUT_ELEMENT_DESC inputElementDescs_[1];
    IDxcUtils* dxcUtils_;
    IDxcIncludeHandler* includeHandler_;
    IDxcCompiler3* dxcCompiler_;
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_;
    ID3DBlob* signatureBlob_;
    ID3DBlob* errorBlob_;
    ID3D12RootSignature* rootSignature_;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_;
    ID3D12PipelineState* graphicsPipelineState_;
    D3D12_BLEND_DESC blendDesc_;
    D3D12_RASTERIZER_DESC rasterizerDesc_;
    D3D12_VIEWPORT viewport_;
    D3D12_RECT scissorRect_;
};

