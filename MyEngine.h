#pragma once
#include "WinApp.h"
#include "Convert.h"
#include <dxcapi.h>
#include "Vector4.h"
#include "Triangle.h"
#include "DirectXManager.h"
#include "externals/DirectXTex/DirectXTex.h"
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
	void Update();
private:
	void IntializeDxcCompiler();
	void CreateRootSignature();
	void BlendSetting();
	void RasiterzerState();
	void CreateGraphicsPipelineState();
	void Viewport();
	DirectX::ScratchImage LoadTex(const std::string& filePath);
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata);
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
	void LoadTexture(const std::string& filePath);
	void SetDepth();
	static DirectXManager* directXManager;
	Triangle* triangle_[10];
	TriangleData triangleVertex_[10];
	Vector4 material[10];
	Matrix4x4 worldMatrix_;
	Transform triangleTransform_;
	Transform cameraTransform_;
	static WinApp* winApp_;
    D3D12_INPUT_ELEMENT_DESC inputElementDescs_[2];
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
	D3D12_ROOT_PARAMETER rootParameters_[3];
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	ID3D12Resource* textureResource_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};
public:
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
};

