#pragma once
#include <chrono>
#include <cstdlib>
#include <dxgi1_6.h>
#include "WinApp.h"
#include "combert.h"
#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")

class DirectXManager
{
public:
	void Initialize(WinApp* win, int32_t backBufferWidth,int32_t backBufferHeight);
	IDxcBlob* CompileShader(const std::wstring& filePath, const wchar_t* profile, IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler, DxcBuffer shaderSourceBuffer, IDxcResult* shaderResult, IDxcBlobEncoding* shaderSource,IDxcBlobUtf8* shaderError);
	void PreDraw();
	void PostDraw();
	void ClearRenderTarget();
	static void Finalize();
	ID3D12Device* GetDevice() { return device_; }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_; }
private:
	static WinApp* winApp_;
	static IDXGIAdapter4* useAdapter_;
	static IDXGIFactory7* dxgiFactory_;
	static ID3D12Device* device_;
	static ID3D12CommandQueue* commandQueue_;
	static ID3D12CommandAllocator* commandAllocator_;
	static ID3D12GraphicsCommandList* commandList_;
	static IDXGISwapChain4* swapChain_;
	static ID3D12DescriptorHeap* rtvDescriptorHeap_;
	static D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];
	static ID3D12Resource* backBuffers_[2];
	static UINT64 fenceVal_;
	static int32_t backBufferWidth_;
	static int32_t backBufferHeight_;
	static inline D3D12_RESOURCE_BARRIER barrier_{};
	static ID3D12Fence* fence_;
	static HANDLE fenceEvent_;
	static HRESULT hr_;
	static IDxcUtils* dxcUtils_;
private:
	void InitializeDXGIDevice();
	void CreateSwapChain();
	void InitializeCommand();
	void CreateFinalRenderTargets();
	void CreateFence();
	void IntializeDxcCompiler(IDxcUtils* dxcUtils, IDxcIncludeHandler* includeHandler, IDxcCompiler3* dxcCompiler);
};
