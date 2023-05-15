#include"WinApp.h"
const char kWindowTitle[] = "CG2_DirectX";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WinApp::CreateWindowView();

	while (WinApp::ProccessMessage() == 0) {

	}
	return 0;
}