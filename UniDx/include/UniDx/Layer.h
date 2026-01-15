#pragma once

#include <cstdint>

namespace UniDx
{

// レイヤー定数（0-31）.
namespace Layer
{
    constexpr int Default = 0;
    constexpr int TransparentFX = 1;
    constexpr int IgnoreRaycast = 2;
    // 3 は予約.
    constexpr int Water = 4;
    constexpr int UI = 5;
    // 6-7 は予約.
    // 8-31 はユーザー定義.
    constexpr int Ground = 8;    // 床/地面用.
    constexpr int Player = 9;    // プレイヤー用.
    constexpr int Enemy = 10;    // 敵用.
}

// レイヤーマスクヘルパー.
namespace LayerMask
{
    constexpr uint32_t GetMask(int layer) { return 1u << layer; }

    template<typename... Layers>
    constexpr uint32_t GetMask(int first, Layers... rest)
    {
        return GetMask(first) | GetMask(rest...);
    }

    constexpr uint32_t Everything = 0xFFFFFFFF;
    constexpr uint32_t Nothing = 0;
}

} // namespace UniDx
