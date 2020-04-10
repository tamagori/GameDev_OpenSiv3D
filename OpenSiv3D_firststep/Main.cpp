
#include "blockBreakingGame.h" 

void Main()
{
	// 使用するフォントアセットを登録
	FontAsset::Register(U"Title", 120, U"example/font/AnnyantRoman/AnnyantRoman.ttf");
	FontAsset::Register(U"Menu", 30, Typeface::Regular);
	FontAsset::Register(U"Score", 36, Typeface::Bold);

	// 背景色を設定
	//Scene::SetBackground(ColorF(0.2, 0.8, 0.4));
	Scene::SetBackground(ColorF(0.0, 0.0, 0.0));

	// シーンマネージャーを作成
	// ここで、CommonDataも初期化される
	// シーンと遷移時の色を設定
	App manager;
	manager
		.add<Title>(State::Title)
		.add<Game>(State::Game)
		.setFadeColor(ColorF(1.0));

	while (System::Update())
	{
		// 現在のシーンを実行
		if (!manager.update())
		{
			break;
		}
	}
}
