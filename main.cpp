#include "MyEngine.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);
	WinApp* win_ =nullptr;
	MyEngine* myEngine = new MyEngine();
	myEngine->Initialize(win_, 1280, 720);
	myEngine->VariableInitialize();
	
	while (true) {
		// メッセージ処理
		if (win_->Procesmessage()) {
			break;
		}
		
		myEngine->BeginFrame();
		//更新処理
		myEngine->Update();
		//描画処理
		myEngine->Draw();
		myEngine->EndFrame();
		
	}
	myEngine->Finalize();
	CoUninitialize();
	return 0;
}