#pragma once
#include <chrono>
#include <cstdlib>
#include <dxgi1_6.h>
#include "WinApp.h"
#include "Convert.h"
#include "Vector4.h"

class Triangle;

class DirectXManager
{
public:
	void Initialize(WinApp* win, int32_t backBufferWidth,int32_t backBufferHeight);
	
	void PreDraw();
	void PostDraw();
	void ClearRenderTarget();
	ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device,D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors,bool shaderVisible);
	void ImguiInitialize();
	static void Finalize();
	ID3D12Device* GetDevice() { return device_; }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_; }
	
public:
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	static ID3D12DescriptorHeap* srvDescriptorHeap_;
private:
	void InitializeDXGIDevice();
	void CreateSwapChain();
	void InitializeCommand();
	void CreateFinalRenderTargets();
	void CreateFence();

};
