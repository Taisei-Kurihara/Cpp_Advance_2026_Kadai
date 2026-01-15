#pragma once

#include <UniDx.h>
#include <UniDx/Rigidbody.h>

using namespace UniDx;

class Player : public UniDx::Behaviour
{
public:
    virtual void OnEnable() override;
    virtual void Update() override;
    virtual void OnTriggerEnter(Collider* other) override;
    virtual void OnTriggerStay(Collider* other) override;
    virtual void OnTriggerExit(Collider* other) override;
    virtual void OnCollisionEnter(const Collision& collision) override;
    virtual void OnCollisionStay(const Collision& collision) override;
    virtual void OnCollisionExit(const Collision& collision) override;

    UniDx::Rigidbody* rb = nullptr;

    // 接地判定.
    bool isGrounded() const { return groundContactCount > 0; }
    void onGroundEnter();
    void onGroundExit();

private:
    float animFrame;
    int groundContactCount = 0;    // 地面との接触数.
    int jumpTime = 0;              // ジャンプ入力時間.
    int unJumpTime = 0;              // ジャンプ入力時間.

    void createGroundCheck();      // 足元トリガー作成.
};
