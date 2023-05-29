#include "DirectXManager.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	DirectXManager* direct = nullptr;

	WinApp* win_ = nullptr;
	direct->Initialize(win_, 1280, 720);

	while (true) {
		direct->PreDraw();
		// メッセージ処理
		if (win_->Procesmessage()) {
			break;
		}

		direct->PostDraw();

	}
	direct->Finalize();
	return 0;
}