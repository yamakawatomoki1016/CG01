#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT Imgui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"d3d12.lib")
class WinApp
{
public:
	static const int32_t GetWidth() { return kClientWidth; }
	static const int32_t GetHeight() { return kClientHeight; }
	static inline HWND GetHwnd() { return hwnd_; }
	HINSTANCE GetHInstance()const { return wc_.hInstance; }
	static	bool Procesmessage();
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static const int32_t kClientWidth = 720;
	static const int32_t kClientHeight = 1280;
public:
	static void CreateGameWindow(
	const wchar_t* title,
	int32_t clientWidth, int32_t clientheight
	);
	static ID3D12Debug1* GetdebugController() { return debugController_; }
private:
	static inline WNDCLASS wc_{};
	static inline RECT wrc_ = { 0,0,kClientHeight,kClientWidth };
	static HWND hwnd_;
	static UINT windowStyle_;
	static ID3D12Debug1* debugController_;
};