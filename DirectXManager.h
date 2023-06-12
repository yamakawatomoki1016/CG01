#pragma once
#include <chrono>
#include <cstdlib>
#include <dxgi1_6.h>
#include "WinApp.h"
#include "Convert.h"
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

class DirectXManager
{
public:

	
private:
	static ID3D12Device* device_;
	static IDXGIAdapter4* useAdapter_;

private:
	void InitializeDXGIFactory();
	void InitializeID3D12Device();
};

