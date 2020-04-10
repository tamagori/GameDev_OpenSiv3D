#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.4.2
#include <HamFramework.hpp>

// シーンの名前
enum class State
{
	Title,
	Game
};

// ゲームデータ
struct GameData
{
	// ハイスコア
	int32 highScore = 0;
};

// シーン管理クラス
using App = SceneManager<State, GameData>;

// タイトルシーン
class Title : public App::Scene
{
private:

	Rect m_startButton = Rect(Arg::center = Scene::Center().moveBy(0, 0), 300, 60);
	Transition m_startTransition = Transition(0.4s, 0.2s);

	Rect m_exitButton = Rect(Arg::center = Scene::Center().moveBy(0, 100), 300, 60);
	Transition m_exitTransition = Transition(0.4s, 0.2s);

public:

	// コンストラクタ（必ず実装）
	Title(const InitData& init)
		: IScene(init) {}

	// 更新関数
	void update() override
	{
		// ボタン上にマウスを置いた場合にアップデートする設定
		m_startTransition.update(m_startButton.mouseOver());
		m_exitTransition.update(m_exitButton.mouseOver());

		// マウスポインタのスタイル変更
		if (m_startButton.mouseOver() || m_exitButton.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}

		// スタートボタン左クリック時にGameへ遷移
		if (m_startButton.leftClicked())
		{
			changeScene(State::Game);
		}

		// exitボタン左クリック時にシステム終了
		if (m_exitButton.leftClicked())
		{
			System::Exit();
		}
	}

	// 描画関数（const 修飾）
	void draw() const override
	{
		const String titleText = U"ブロックくずし";
		const Vec2 center(Scene::Center().x, 120);

		// タイトル描画
		FontAsset(U"Title")(titleText).drawAt(center.movedBy(4, 6), ColorF(0.0, 0.5));
		FontAsset(U"Title")(titleText).drawAt(center);

		// ボタン上にマウスポインタを置いた場合にボタンの色を変える
		m_startButton.draw(ColorF(1.0, m_startTransition.value())).drawFrame(2);
		m_exitButton.draw(ColorF(1.0, m_exitTransition.value())).drawFrame(2);

		// ボタンのデザイン設定
		FontAsset(U"Menu")(U"はじめる").drawAt(m_startButton.center(), ColorF(0.25));
		FontAsset(U"Menu")(U"おわる").drawAt(m_exitButton.center(), ColorF(0.25));

		// おそらく背景色の設定
		Rect(0, 500, Scene::Width(), Scene::Height() - 500)
			.draw(Arg::top = ColorF(0.0, 0.0), Arg::bottom = ColorF(0.0, 0.5));

		// スコアを表示
		const int32 highScore = getData().highScore;
		FontAsset(U"Score")(U"High Score: {}"_fmt(highScore)).drawAt(Vec2(620, 550));
	}
};

// ゲームシーン
class Game : public App::Scene
{
private:

	// ブロックのサイズ
	static constexpr Size blockSize = Size(40, 20);

	// ボールの速さ
	static constexpr double speed = 480.0;

	// ブロックの配列
	Array<Rect> m_blocks;

	// ボールの速度
	Vec2 m_ballVelocity = Vec2(0, -speed);

	// ボール
	Circle m_ball = Circle(400, 400, 8);

	// パドル
	Rect m_paddle = Rect(Arg::center(Cursor::Pos().x, 500), 60, 10);

	// スコア
	int32 m_score = 0;

	// 2D パーティクルシステム
	ParticleSystem2D m_system;

	// パーティクル用のテクスチャ
	Texture m_texture;

public:

	Game(const InitData& init)
		: IScene(init)
		, m_texture(U"example/particle.png", TextureDesc::Mipped)
	{
		// 横（Scene::Width() / blockSize.x）個、縦 5 個のブロックを配列に追加する
		for (auto p : step(Size((Scene::Width() / blockSize.x), 5)))
		{
			m_blocks << Rect(p.x * blockSize.x, 60 + p.y * blockSize.y, blockSize);
		}

		// パーティクルシステムの位置と力ベクトルを設定
		m_system.setPosition(m_ball.center);
		m_system.setForce(Vec2(0.0, -120));

		// パラメータ
		ParticleSystem2DParameters parameters;
		parameters.rate = 300.0;
		parameters.startSpeed = 50.0;
		parameters.startColor = HSV(0.0, 0.8, 0.25);
		parameters.startSize = 30.0;
		parameters.blendState = BlendState::Additive;
		m_system.setParameters(parameters);

		// エミッター
		CircleEmitter2D circleEmitter;
		circleEmitter.r = 12.0;
		m_system.setEmitter(circleEmitter);

		m_system.prewarm();
	}

	void update() override
	{
		// パドルを操作
		m_paddle = Rect(Arg::center(Cursor::Pos().x, 500), 60, 10);

		// ボールを移動
		m_ball.moveBy(m_ballVelocity * Scene::DeltaTime());

		// ブロックを順にチェック
		for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it)
		{
			// ボールとブロックが交差していたら
			if (it->intersects(m_ball))
			{
				// ボールの向きを反転する
				(it->bottom().intersects(m_ball) || it->top().intersects(m_ball) ? m_ballVelocity.y : m_ballVelocity.x) *= -1;

				// ブロックを配列から削除（イテレータが無効になるので注意）
				m_blocks.erase(it);

				// スコアを加算
				++m_score;

				// これ以上チェックしない  
				break;
			}
		}

		// 天井にぶつかったらはね返る
		if (m_ball.y < 0 && m_ballVelocity.y < 0)
		{
			m_ballVelocity.y *= -1;
		}

		if (m_ball.y > Scene::Height())
		{
			changeScene(State::Title);
			getData().highScore = Max(getData().highScore, m_score);
		}

		// 左右の壁にぶつかったらはね返る
		if ((m_ball.x < 0 && m_ballVelocity.x < 0) || (Scene::Width() < m_ball.x && m_ballVelocity.x > 0))
		{
			m_ballVelocity.x *= -1;
		}

		// パドルにあたったらはね返る
		if (m_ballVelocity.y > 0 && m_paddle.intersects(m_ball))
		{
			// パドルの中心からの距離に応じてはね返る向きを変える
			m_ballVelocity = Vec2((m_ball.x - m_paddle.center().x) * 10, -m_ballVelocity.y).setLength(speed);
		}

		// パーティクルの位置更新
		m_system.setPosition(m_ball.center);
		m_system.update();
	}

	void draw() const override
	{
		FontAsset(U"Score")(m_score).drawAt(Scene::Center().x, 30);

		// すべてのブロックを描画する
		for (const auto& block : m_blocks)
		{
			block.stretched(-1).draw(HSV(block.y - 40));
		}

		// ボールを描く
		m_ball.draw();

		// パドルを描く
		m_paddle.draw();

		// パーティクル描画
		m_system.draw();
	}
};