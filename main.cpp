#include "MyEngine.h"
#include "Sprite.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);
	WinApp* win_ =nullptr;
	MyEngine* myEngine = new MyEngine();
	Sprite* sprite = new Sprite();
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
		sprite->Draw();
	}
	myEngine->Finalize();
	CoUninitialize();
	return 0;
}