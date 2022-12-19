#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include <random>

using namespace DirectX;
using namespace std;

GameScene::GameScene() {}

GameScene::~GameScene() { delete model_; }

void GameScene::Initialize()
{
	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();
	debugText_ = DebugText::GetInstance();

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("mario.jpg");

	// 3Dモデルの生成
	model_ = Model::Create();

	// キャラクターの大元
	worldTransform_[Root].Initialize();
	// 脊椎
	worldTransform_[PartId::Spine].translation_ = {0, 0, 0};
	worldTransform_[PartId::Spine].parent_ = &worldTransform_[PartId::Root];
	worldTransform_[PartId::Spine].Initialize();
	// 上半身
	// Chest
	worldTransform_[PartId::Chest].translation_ = {0, 0.0f, 0};
	worldTransform_[PartId::Chest].parent_ = &worldTransform_[PartId::Spine];
	worldTransform_[PartId::Chest].Initialize();
	// Head
	worldTransform_[PartId::Head].translation_ = {0, 2.5f, 0};
	worldTransform_[PartId::Head].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::Head].Initialize();
	// ArmL
	worldTransform_[PartId::ArmL].translation_ = {-2.5f, 0, 0};
	worldTransform_[PartId::ArmL].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::ArmL].Initialize();
	// ArmR
	worldTransform_[PartId::ArmR].translation_ = {2.5f, 0, 0};
	worldTransform_[PartId::ArmR].parent_ = &worldTransform_[PartId::Chest];
	worldTransform_[PartId::ArmR].Initialize();

	// 下半身
	// Hip
	worldTransform_[PartId::Hip].translation_ = {0, -2.5f, 0};
	worldTransform_[PartId::Hip].parent_ = &worldTransform_[PartId::Spine];
	worldTransform_[PartId::Hip].Initialize();
	// LegL
	worldTransform_[PartId::LegL].translation_ = {-2.5f, -2.5f, 0};
	worldTransform_[PartId::LegL].parent_ = &worldTransform_[PartId::Hip];
	worldTransform_[PartId::LegL].Initialize();
	// LegR
	worldTransform_[PartId::LegR].translation_ = {2.5f, -2.5f, 0};
	worldTransform_[PartId::LegR].parent_ = &worldTransform_[PartId::Hip];
	worldTransform_[PartId::LegR].Initialize();

	// ビュープロジェクション初期化
	viewProjection_.Initialize();
}

void GameScene::Update()
{	
	// キャラクター移動処理
	{
		// キャラクターの移動ベクトル
		XMFLOAT3 move = {0, 0, 0};
		// キャラクターの移動速度
		const float kCharactoerSpeed = 0.2f;
		// 押した方向で移動ベクトルを変更
		if (input_->PushKey(DIK_LEFT)) {
			move = {-kCharactoerSpeed, 0, 0};
		} else if (input_->PushKey(DIK_RIGHT)) {
			move = {kCharactoerSpeed, 0, 0};
		}
		if (input_->PushKey(DIK_DOWN)) {
			move = { 0, -kCharactoerSpeed, 0 };
		}
		else if (input_->PushKey(DIK_UP)) {
			move = { 0, kCharactoerSpeed, 0 };
		}

		// 注視点移動（ベクトルの加算）
		worldTransform_[PartId::Root].translation_.x += move.x;
		worldTransform_[PartId::Root].translation_.y += move.y;
		worldTransform_[PartId::Root].translation_.z += move.z;

		// デバッグ用表示
		debugText_->SetPos(50, 50);
		debugText_->Printf(
		  "Root: %f,%f,%f",
		  worldTransform_[PartId::Root].translation_.x,
		  worldTransform_[PartId::Root].translation_.y,
		  worldTransform_[PartId::Root].translation_.z);
	}
	// 上半身回転処理
	{
		// 上半身の回転速度[ラジアン/frame]
		const float kChestRotSpeed = 0.05f;
		// 押した方向で移動ベクトルを変更
		if (input_->PushKey(DIK_U)) {
			worldTransform_[Chest].rotation_.y -= kChestRotSpeed;
		} else if (input_->PushKey(DIK_I)) {
			worldTransform_[Chest].rotation_.y += kChestRotSpeed;
		}
	}
	// 下半身回転処理
	{
		// 下半身の回転速度[ラジアン/frame]
		const float kHipRotSpeed = 0.05f;
		// 押した方向で移動ベクトルを変更
		if (input_->PushKey(DIK_J)) {
			worldTransform_[Hip].rotation_.y -= kHipRotSpeed;
		} else if (input_->PushKey(DIK_K)) {
			worldTransform_[Hip].rotation_.y += kHipRotSpeed;
		}
	}

	worldTransform_[PartId::Root].UpdateMatrix();
	worldTransform_[PartId::Spine].UpdateMatrix();
	worldTransform_[PartId::Chest].UpdateMatrix();
	worldTransform_[PartId::Head].UpdateMatrix();
	worldTransform_[PartId::Hip].UpdateMatrix();
	worldTransform_[PartId::ArmL].UpdateMatrix();
	worldTransform_[PartId::ArmR].UpdateMatrix();
	worldTransform_[PartId::LegL].UpdateMatrix();
	worldTransform_[PartId::LegR].UpdateMatrix();
}

void GameScene::Draw()
{
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>
	
	// 3Dモデルの描画
	/*model_->Draw(worldTransform_[PartId::Root], viewProjection_, textureHandle_);*/
	/*model_->Draw(worldTransform_[PartId::Spine], viewProjection_, textureHandle_);*/
	model_->Draw(worldTransform_[PartId::Chest], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Head], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::ArmL], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::ArmR], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::Hip], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::LegL], viewProjection_, textureHandle_);
	model_->Draw(worldTransform_[PartId::LegR], viewProjection_, textureHandle_);

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// デバッグテキストの描画
	debugText_->DrawAll(commandList);
	//
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}
