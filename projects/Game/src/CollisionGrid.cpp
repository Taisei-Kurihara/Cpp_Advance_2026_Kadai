#include "CollisionGrid.h"
#include <cmath>
#include <algorithm>

CollisionGrid::CellCoord CollisionGrid::worldToCell(Vector3 pos) const
{
    int x = static_cast<int>(std::floor(pos.x / cellSize.x));
    int y = static_cast<int>(std::floor(pos.y / cellSize.y));
    int z = static_cast<int>(std::floor(pos.z / cellSize.z));
    return { x, y, z };
}


void CollisionGrid::enableCollider(Collider* col)
{
    if (col && registeredColliders_.find(col) == registeredColliders_.end())
    {
        Physics::getInstance()->register3d(col);
        registeredColliders_.insert(col);
    }
}


void CollisionGrid::disableCollider(Collider* col)
{
    if (col && registeredColliders_.find(col) != registeredColliders_.end())
    {
        Physics::getInstance()->unregister3d(col);
        registeredColliders_.erase(col);
    }
}


void CollisionGrid::registerStatic(Collider* collider)
{
    if (collider == nullptr) return;

    // コライダーの位置からセル座標を計算.
    Vector3 pos = collider->transform->position;
    CellCoord cell = worldToCell(pos);

    // セルに登録.
    staticColliders_[cell].push_back(collider);

    // 初期状態ではPhysicsから解除（OnEnableで登録されているので解除）.
    Physics::getInstance()->unregister3d(collider);
}


void CollisionGrid::scanAndRegister(Transform* t)
{
    if (t == nullptr) return;

    // このTransformのGameObjectからAABBColliderを取得.
    auto col = t->gameObject->GetComponent<AABBCollider>(false);
    if (col != nullptr)
    {
        registerStatic(col);
    }

    // 子を再帰的にスキャン.
    for (const auto& child : t->getChildGameObjects())
    {
        scanAndRegister(child->transform);
    }
}


void CollisionGrid::OnEnable()
{
    // 初期化は最初のUpdateで行う（mapRootがセットされるまで待つ）.
}


void CollisionGrid::RegisterDynamic(Transform* transform)
{
    if (transform == nullptr) return;
    dynamicObjects_.push_back(transform);
}


void CollisionGrid::UnregisterDynamic(Transform* transform)
{
    auto it = std::find(dynamicObjects_.begin(), dynamicObjects_.end(), transform);
    if (it != dynamicObjects_.end())
    {
        dynamicObjects_.erase(it);
    }
}


void CollisionGrid::Update()
{
    // 初回のみ初期化処理を行う.
    if (!initialized_ && mapRoot != nullptr)
    {
        scanAndRegister(mapRoot);
        initialized_ = true;
    }

    // 動的オブジェクトがない場合は処理しない.
    if (dynamicObjects_.empty()) return;

    // 現在有効なセルをクリア.
    activeCells_.clear();

    // 各動的オブジェクトの周囲セルを有効セットに追加.
    for (Transform* dynObj : dynamicObjects_)
    {
        if (dynObj == nullptr) continue;

        Vector3 pos = dynObj->position;
        CellCoord centerCell = worldToCell(pos);
        auto [cx, cy, cz] = centerCell;

        // 周囲のセルを有効化.
        for (int dx = -activeRadius; dx <= activeRadius; ++dx)
        {
            for (int dy = -activeRadius; dy <= activeRadius; ++dy)
            {
                for (int dz = -activeRadius; dz <= activeRadius; ++dz)
                {
                    activeCells_.insert({ cx + dx, cy + dy, cz + dz });
                }
            }
        }
    }

    // 前フレームから無効になったセルのコライダーを無効化.
    for (const CellCoord& cell : prevActiveCells_)
    {
        if (activeCells_.find(cell) == activeCells_.end())
        {
            // このセルは無効になった.
            auto it = staticColliders_.find(cell);
            if (it != staticColliders_.end())
            {
                for (Collider* col : it->second)
                {
                    disableCollider(col);
                }
            }
        }
    }

    // 新しく有効になったセルのコライダーを有効化.
    for (const CellCoord& cell : activeCells_)
    {
        if (prevActiveCells_.find(cell) == prevActiveCells_.end())
        {
            // このセルは新しく有効になった.
            auto it = staticColliders_.find(cell);
            if (it != staticColliders_.end())
            {
                for (Collider* col : it->second)
                {
                    enableCollider(col);
                }
            }
        }
    }

    // 現在のセットを前フレームとして保存.
    prevActiveCells_ = activeCells_;
}
