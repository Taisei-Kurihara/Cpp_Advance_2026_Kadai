// CreateDefaultScene.cpp
// デフォルトのシーンを生成します

#include <numbers>

#include <UniDx.h>
#include <UniDx/Scene.h>
#include <UniDx/PrimitiveRenderer.h>
#include <UniDx/GltfModel.h>
#include <UniDx/Canvas.h>
#include <UniDx/TextMesh.h>
#include <UniDx/Font.h>
#include <UniDx/Image.h>
#include <UniDx/LightManager.h>
#include <UniDx/Layer.h>

#include "CameraController.h"
#include "Player.h"
#include "MapData.h"
#include "MapBuilder.h"
#include "LightController.h"

#include <thread>
#include "MainGame.h"


using namespace std;
using namespace UniDx;


void MainGame::createMap()
{
    // マテリアルの作成.
    auto wallMat = std::make_shared<Material>();
    auto floorMat = std::make_shared<Material>();
    auto coinMat = std::make_shared<Material>();

    // シェーダを指定してコンパイル.
    wallMat->shader->compile<VertexPNT>(u8"resource/AlbedoShadeSpec.hlsl");
    floorMat->shader->compile<VertexPNT>(u8"resource/AlbedoShadeSpec.hlsl");
    floorMat->color = Color(0.85f, 0.8f, 0.85f);
    coinMat->shader->compile<VertexPN>(u8"resource/ShadeSpec.hlsl");
    coinMat->color = Color(1.0f, 0.9f, 0.1f);

    // 床テクスチャ作成.
    auto floorTex = std::make_shared<Texture>();
    floorTex->Load(u8"resource/floor.png");
    floorMat->AddTexture(std::move(floorTex));

    // 壁テクスチャ作成.
    auto wallTex = std::make_shared<Texture>();
    wallTex->Load(u8"resource/wall.png");
    wallMat->AddTexture(std::move(wallTex));

    // MapBuilder設定.
    MapBuilder builder;
    builder
        .setOrigin(Vector3(0, -1.5f, 50))   // Y=-1.5を基点（床の高さ）.
        .setAlignment(Vector3(0, -1, 0))   // X,Z=中央揃え、Y=下揃え.
        .setSpacing(Vector3(2, 1.5f, 2))   // X,Z=2間隔、Y=1.5間隔（床→壁）.
        .setDefaultSize(Vector3(2, 2, 2)); // デフォルトサイズ.

    // 床オブジェクト登録 'y'.
    // layerMaskでPlayer/Enemyとのみ衝突判定（床同士の判定をスキップして軽量化）.
    builder.Register('y', [&floorMat](Vector3 pos, Vector3 size) {
        auto rb = make_unique<Rigidbody>();
        rb->gravityScale = 0;
        rb->mass = numeric_limits<float>::infinity();

        auto floorCollider = make_unique<AABBCollider>();
        floorCollider->layer = Layer::Ground;
        floorCollider->size = Vector3(1.1f, 0.5f, 1.1f);
        floorCollider->bounciness = 0.0f;
        // Player/Enemyレイヤーとのみ衝突判定.
        floorCollider->layerMask = LayerMask::GetMask(Layer::Player, Layer::Enemy);

        auto floor = make_unique<GameObject>(u8"床",
            CubeRenderer::create<VertexPNT>(floorMat),
            move(rb),
            move(floorCollider));
        floor->transform->localScale = Vector3(4, 1, 4);
        floor->setLayer(Layer::Ground);
        floor->setTag(u8"Ground");
        return floor;
    });

    // 壁オブジェクト登録 'w'.
    // layerMaskでPlayer/Enemyとのみ衝突判定（壁同士の判定をスキップして軽量化）.
    builder.Register('w', [&wallMat](Vector3 pos, Vector3 size) {
        auto rb = make_unique<Rigidbody>();
        rb->gravityScale = 0;
        rb->mass = numeric_limits<float>::infinity();

        auto wallCollider = make_unique<AABBCollider>();
        wallCollider->size = Vector3(0.5f, 0.5f, 0.5f);
        wallCollider->bounciness = 0.0f;
        // Player/Enemyレイヤーとのみ衝突判定.
        wallCollider->layerMask = LayerMask::GetMask(Layer::Player, Layer::Enemy);

        auto wall = make_unique<GameObject>(u8"壁",
            CubeRenderer::create<VertexPNT>(wallMat),
            move(rb),
            move(wallCollider));
        wall->transform->localScale = Vector3(2, 2, 2);
        return wall;
    });

    // コインオブジェクト登録 'c'.
    builder.Register('c', [&coinMat](Vector3 pos, Vector3 size) {
        auto coin = make_unique<GameObject>(u8"Coin",
            make_unique<GltfModel>(),
            make_unique<Rigidbody>(),
            make_unique<SphereCollider>(Vector3(0, -0.1f, 0), 0.4f));
        auto model = coin->GetComponent<GltfModel>(true);
        model->Load<VertexPN>(u8"resource/coin.glb", coinMat);
        coin->transform->localScale = Vector3(3, 3, 3);
        return coin;
    });

    // 3Dマップデータ [高さ][Z行][X列] - 90度回転済み.
    // y=床, w=壁, c=コイン, _=空白.
    // 床は4x4サイズなので1つおきに配置（境界を減らす）.
    std::vector<std::vector<std::string>> mapData = {
        // 高さ0（床レベル）- Z方向に展開.
        {
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y",
            "_______________",
            "y_y_y_y_y_y_y_y"
        }
        ,
        // 高さ1（壁・コインレベル）- Z方向に展開.
        {
            "wwwwwwwwwwwwwww",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "wwwwwwwwwwwwwww"
        }
        ,
        // 高さ2（壁・コインレベル）- Z方向に展開.
        {
            "wwwwwwwwwwwwwww",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "wwwwwwwwwwwwwww"
        }
        ,
        // 高さ3（壁・コインレベル）- Z方向に展開.
        {
            "wwwwwwwwwwwwwww",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "w_____________w",
            "wwwwwwwwwwwwwww"
        }
    };
    //(既)修: layerMaskでPlayer/Enemyとのみ衝突判定（床・壁同士の判定をスキップして軽量化）.

    mapObj = builder.Build(mapData);
}


unique_ptr<UniDx::Scene> MainGame::CreateScene()
{
    // -- プレイヤー --
    auto playerObj = make_unique<GameObject>(u8"プレイヤー",
        make_unique<GltfModel>(),
        make_unique<Rigidbody>(),
        make_unique<SphereCollider>(Vector3(0, 0.25f,0.0f)),
        make_unique<Player>()
        );
    auto model = playerObj->GetComponent<GltfModel>(true);
    model->Load<VertexSkin>(
        u8"resource/mini_emma.glb",
        u8"resource/SkinBasic.hlsl");
    playerObj->transform->localPosition = Vector3(0, 5, 0);
    playerObj->transform->localRotation = Quaternion::Euler(0, 180, 0);

    // -- カメラ --
    auto cameraBehaviour = make_unique<CameraController>();
    cameraBehaviour->player = playerObj->GetComponent<Player>(true);

    // -- ライト --
    LightManager::getInstance()->ambientColor = Color(0.3f, 0.3f, 0.3f, 1.0f);

    auto lights = make_unique<GameObject>(u8"ライト群");
    auto light = make_unique<GameObject>(u8"ディレクショナルライト", make_unique<Light>(), make_unique<LightController>());
    light->transform->localPosition = Vector3(4, 3, 0);
    light->GetComponent<Light>(true)->intensity = 0.4f;
    Transform::SetParent(move(light), lights->transform);

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            auto l = make_unique<Light>();
            l->type = LightType_Point;
            l->range = 10.0f;

            auto light = make_unique<GameObject>(u8"ポイントライト",
                move(l),
                make_unique<LightController>());
            light->transform->localPosition = Vector3(10.0f * j - 5.0f, 4,  10.0f * i - 5.0f);
            Transform::SetParent(move(light), lights->transform);
        }
    }

    // -- UI --
    auto font = make_shared<Font>();
    font->Load(u8"resource/M PLUS 1.spritefont");
    auto textMesh = make_unique<TextMesh>();
    textMesh->font = font;
    textMesh->text = u8"WASD:いどう\nIJKL:カメラ\nOP:ライト";

    auto textObj = make_unique<GameObject>(u8"テキスト", textMesh);
    textObj->transform->localPosition = Vector3(100, 20, 0);
    textObj->transform->localScale = Vector3(0.6f, 0.6f, 1.0f);

    auto scoreMesh = make_unique<TextMesh>();
    scoreMesh->font = font;
    scoreMesh->text = u8"0";
    scoreTextMesh = scoreMesh.get();

    auto scoreTextObj = make_unique<GameObject>(u8"スコア", scoreMesh);
    scoreTextObj->transform->localPosition = Vector3(480, 20, 0);

    auto canvas = make_unique<Canvas>();
    canvas->LoadDefaultMaterial(u8"resource");

    // -- マップデータ --
    createMap();

    // シーンを作って戻す
    return make_unique<Scene>(

        make_unique<GameObject>(u8"オブジェクトルート",
            move(playerObj),
            move(mapObj)
        ),

        move(lights),

        make_unique<GameObject>(u8"カメラルート", Vector3(0, 3, -5),
            make_unique<Camera>(),
            move(cameraBehaviour)
        ),

        make_unique<GameObject>(u8"UI",
            move(canvas),
            move(textObj),
            move(scoreTextObj)
        )
    );
}


MainGame::~MainGame()
{
}


void MainGame::AddScore(int n)
{
    score += n;
    scoreTextMesh->text = ToString(score);
}
