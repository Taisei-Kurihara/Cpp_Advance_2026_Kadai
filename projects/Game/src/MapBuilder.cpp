#include "MapBuilder.h"
#include <algorithm>

MapBuilder::MapBuilder()
{
}

MapBuilder& MapBuilder::setOrigin(Vector3 origin)
{
    origin_ = origin;
    return *this;
}

MapBuilder& MapBuilder::setAlignment(Vector3 alignment)
{
    alignment_ = alignment;
    return *this;
}

MapBuilder& MapBuilder::setSpacing(Vector3 spacing)
{
    spacing_ = spacing;
    return *this;
}

MapBuilder& MapBuilder::setDefaultSize(Vector3 size)
{
    defaultSize_ = size;
    return *this;
}

MapBuilder& MapBuilder::Register(char c, ObjectFactory factory)
{
    factories_[c] = std::move(factory);
    return *this;
}

MapBuilder& MapBuilder::addIgnoreChar(char c)
{
    if (std::find(ignoreChars_.begin(), ignoreChars_.end(), c) == ignoreChars_.end())
    {
        ignoreChars_.push_back(c);
    }
    return *this;
}

bool MapBuilder::isIgnoreChar(char c) const
{
    return std::find(ignoreChars_.begin(), ignoreChars_.end(), c) != ignoreChars_.end();
}

Vector3 MapBuilder::calculatePosition(int gridX, int gridY, int gridZ, Vector3 mapSize) const
{
    // アライメントに応じたオフセット計算.
    // alignment = 0: 中央揃え → オフセット = (mapSize - 1) / 2.
    // alignment = -1: 負方向端揃え → オフセット = 0.
    // alignment = 1: 正方向端揃え → オフセット = mapSize - 1.
    float offsetX = (mapSize.x - 1.0f) * (alignment_.x + 1.0f) * 0.5f;
    float offsetY = (mapSize.y - 1.0f) * (alignment_.y + 1.0f) * 0.5f;
    float offsetZ = (mapSize.z - 1.0f) * (alignment_.z + 1.0f) * 0.5f;

    // グリッド座標からワールド座標へ変換.
    Vector3 worldPos{
        (static_cast<float>(gridX) - offsetX) * spacing_.x,
        (static_cast<float>(gridY) - offsetY) * spacing_.y,
        (static_cast<float>(gridZ) - offsetZ) * spacing_.z
    };

    return origin_ + worldPos;
}

std::unique_ptr<GameObject> MapBuilder::Build(const std::vector<std::vector<std::string>>& mapData)
{
    // マップサイズの計算.
    size_t heightCount = mapData.size();
    size_t maxRowCount = 0;
    size_t maxColCount = 0;

    for (const auto& layer : mapData)
    {
        maxRowCount = std::max(maxRowCount, layer.size());
        for (const auto& row : layer)
        {
            maxColCount = std::max(maxColCount, row.size());
        }
    }

    Vector3 mapSize{
        static_cast<float>(maxColCount),  // X軸: 列数.
        static_cast<float>(heightCount),   // Y軸: レイヤー数.
        static_cast<float>(maxRowCount)    // Z軸: 行数.
    };

    // ルートGameObjectを作成.
    auto root = std::make_unique<GameObject>(u8"MapRoot ");

    // 各レイヤー（高さ）を処理.
    for (size_t y = 0; y < mapData.size(); ++y)
    {
        const auto& layer = mapData[y];

        // 各行（Z軸）を処理.
        for (size_t z = 0; z < layer.size(); ++z)
        {
            const auto& row = layer[z];

            // 各列（X軸）を処理.
            for (size_t x = 0; x < row.size(); ++x)
            {
                char c = row[x];

                // 無視文字はスキップ.
                if (isIgnoreChar(c))
                {
                    continue;
                }

                // ファクトリが登録されているか確認.
                auto it = factories_.find(c);
                if (it == factories_.end())
                {
                    // 未登録の文字はスキップ.
                    continue;
                }

                // 位置を計算.
                // Z軸は行番号をそのまま使用（アライメントで中央揃えされる）.
                Vector3 position = calculatePosition(
                    static_cast<int>(x),
                    static_cast<int>(y),
                    static_cast<int>(z),
                    mapSize
                );

                // オブジェクトを生成.
                auto obj = it->second(position, defaultSize_);

                if (obj)
                {
                    // 位置を設定.
                    obj->transform->localPosition = position;

                    // スケールはファクトリで設定されていなければデフォルトを使用.
                    // （ファクトリ内で設定済みの場合は上書きしない）.

                    // 親子関係を設定.
                    Transform::SetParent(std::move(obj), root->transform);
                }
            }
        }
    }

    return root;
}
