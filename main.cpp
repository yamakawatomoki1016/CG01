#include "MyEngine.h"
#include "Sphere.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	CoInitializeEx(0, COINIT_MULTITHREADED);;
	WinApp* win_ =nullptr;
	MyEngine* myEngine = new MyEngine();
	Sphere* sphere_ = new Sphere();
	myEngine->Initialize(win_, 1280, 720);
	myEngine->VariableInitialize();
	sphere_->Initialize(myEngine->GetDirectXCommon(),myEngine);
	Vector4 material = { 1.0f,1.0f,1.0f,1.0f };
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
		sphere_->Draw(material,myEngine->GetWorldTransform());
		myEngine->EndFrame();
	}
	sphere_->Finalize();
	myEngine->Finalize();
	CoUninitialize();
	return 0;
}