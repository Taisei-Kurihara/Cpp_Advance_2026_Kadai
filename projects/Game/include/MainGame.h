#pragma once

#include <UniDx.h>

using namespace UniDx;

namespace UniDx
{
    class Scene;
    class TextMesh;
}

class CollisionGrid;

class MainGame : public UniDx::Singleton<MainGame>
{
public:
    virtual ~MainGame();

    void AddScore(int n);

    unique_ptr<UniDx::Scene> CreateScene();

    // グリッドベース当たり判定システム.
    CollisionGrid* collisionGrid = nullptr;

protected:
    int score = 0;
    unique_ptr<UniDx::GameObject> mapObj;
    UniDx::TextMesh* scoreTextMesh;

    void createMap();

    // マップの静的コライダーをグリッドに登録.
    void registerMapColliders(Transform* transform);
};