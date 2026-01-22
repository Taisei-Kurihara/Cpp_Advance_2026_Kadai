#pragma once

#include <UniDx.h>
#include <UniDx/Collider.h>
#include <UniDx/Physics.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace UniDx;

// グリッドベースの当たり判定最適化システム.
// 動的オブジェクトが存在するセルとその周囲のみ当たり判定を有効化.
class CollisionGrid : public Behaviour
{
public:
    // グリッド設定.
    Vector3 cellSize = Vector3(8.0f, 8.0f, 8.0f);  // セルサイズ.
    int activeRadius = 1;  // 有効化する周囲セル数.

    // スキャン対象のルートTransform.
    Transform* mapRoot = nullptr;

    // 動的オブジェクトを登録.
    void RegisterDynamic(Transform* transform);

    // 動的オブジェクトを解除.
    void UnregisterDynamic(Transform* transform);

    virtual void OnEnable() override;
    virtual void Update() override;

private:
    // セル座標をハッシュ化.
    struct CellHash
    {
        size_t operator()(const std::tuple<int, int, int>& cell) const
        {
            auto [x, y, z] = cell;
            return std::hash<int>()(x) ^ (std::hash<int>()(y) << 1) ^ (std::hash<int>()(z) << 2);
        }
    };

    using CellCoord = std::tuple<int, int, int>;

    // ワールド座標からセル座標を計算.
    CellCoord worldToCell(Vector3 pos) const;

    // コライダーを有効化（Physicsに登録）.
    void enableCollider(Collider* col);

    // コライダーを無効化（Physicsから解除）.
    void disableCollider(Collider* col);

    // 静的コライダーを登録（Physicsから一旦解除）.
    void registerStatic(Collider* collider);

    // 再帰的にコライダーをスキャンして登録.
    void scanAndRegister(Transform* t);

    // セルごとの静的コライダーリスト.
    std::unordered_map<CellCoord, std::vector<Collider*>, CellHash> staticColliders_;

    // 動的オブジェクトのTransformリスト.
    std::vector<Transform*> dynamicObjects_;

    // 現在有効なセル.
    std::unordered_set<CellCoord, CellHash> activeCells_;

    // 前フレームで有効だったセル.
    std::unordered_set<CellCoord, CellHash> prevActiveCells_;

    // 現在Physicsに登録されているコライダー.
    std::unordered_set<Collider*> registeredColliders_;

    // 初期化済みフラグ.
    bool initialized_ = false;
};
