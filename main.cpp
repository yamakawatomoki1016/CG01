#include "MyEngine.h"
#include "Triangle.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
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
	return 0;
}