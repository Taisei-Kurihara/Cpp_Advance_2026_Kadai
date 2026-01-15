#pragma once

#include <UniDx.h>
#include <UniDx/Layer.h>

using namespace UniDx;

class Player;  // 前方宣言.

// 足元トリガー用のBehaviour.
class GroundCheck : public Behaviour
{
public:
    Player* player = nullptr;  // 親Playerへの参照.

    virtual void OnTriggerEnter(Collider* other) override;
    virtual void OnTriggerExit(Collider* other) override;
};
