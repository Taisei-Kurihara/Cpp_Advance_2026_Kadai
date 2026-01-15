#pragma once

#include <UniDx.h>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

using namespace UniDx;

// オブジェクト生成用ファクトリ関数の型定義.
// pos: 配置位置（ワールド座標）, size: オブジェクトのサイズ.
using ObjectFactory = std::function<std::unique_ptr<GameObject>(Vector3 pos, Vector3 size)>;

// 3Dマップビルダークラス.
// 文字配列ベースの3DマップデータからGameObject階層を生成する.
class MapBuilder
{
public:
    MapBuilder();
    ~MapBuilder() = default;

    // コピー禁止、ムーブ許可.
    MapBuilder(const MapBuilder&) = delete;
    MapBuilder& operator=(const MapBuilder&) = delete;
    MapBuilder(MapBuilder&&) = default;
    MapBuilder& operator=(MapBuilder&&) = default;

    // 基点位置を設定.
    MapBuilder& setOrigin(Vector3 origin);

    // アライメント（揃え方向）を設定.
    // 各軸: -1=負方向端, 0=中央, 1=正方向端.
    MapBuilder& setAlignment(Vector3 alignment);

    // オブジェクト間の間隔を設定.
    MapBuilder& setSpacing(Vector3 spacing);

    // デフォルトのオブジェクトサイズを設定.
    MapBuilder& setDefaultSize(Vector3 size);

    // 文字に対応するオブジェクト生成関数を登録.
    MapBuilder& Register(char c, ObjectFactory factory);

    // 空白文字を追加登録.
    MapBuilder& addIgnoreChar(char c);

    // 3Dマップデータからオブジェクトツリーを生成.
    // mapData: [height][row][column] の3D配列.
    // '_' と ' ' はデフォルトで空白として無視される.
    std::unique_ptr<GameObject> Build(const std::vector<std::vector<std::string>>& mapData);

private:
    Vector3 origin_{ 0, 0, 0 };
    Vector3 alignment_{ 0, 0, 0 };
    Vector3 spacing_{ 1, 1, 1 };
    Vector3 defaultSize_{ 1, 1, 1 };

    std::unordered_map<char, ObjectFactory> factories_;
    std::vector<char> ignoreChars_{ '_', ' ' };

    // グリッド座標からワールド座標を計算.
    Vector3 calculatePosition(int gridX, int gridY, int gridZ, Vector3 mapSize) const;

    // 文字が無視対象かどうかを判定.
    bool isIgnoreChar(char c) const;
};
