#include "WinApp.h"

//Windowsアプリでのエントリーポイント（main関数）
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	WinApp::CreateWindowView();

	while (WinApp::ProccessMessage() == 0) {
		
	}

	return 0;
}