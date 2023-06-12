#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public:
	static const int32_t kClientWindth = 1280;
	static const int32_t kClientHeight = 720;
	//ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT mag,WPARAM wparam, LPARAM lparam);
	static void CreateWindowView();
	static int ProccessMessage();
	
private:
	static inline WNDCLASS wc{};
	static HWND hwnd;
};

