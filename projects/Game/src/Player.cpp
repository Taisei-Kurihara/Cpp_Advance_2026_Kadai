#pragma once

#include "Player.h"
#include "GroundCheck.h"

#include <UniDx/Input.h>
#include <UniDx/Collider.h>
#include <UniDx/Time.h>
#include <UniDx/Layer.h>

#include "MainGame.h"

#include <cmath>

using namespace DirectX;
using namespace UniDx;

namespace
{
}


void Player::OnEnable()
{
    rb = GetComponent<Rigidbody>(true);
    assert(rb != nullptr);

    rb->gravityScale = 30.0f;
    animFrame = 0.0f;

    // プレイヤー本体のコライダー設定.
    auto col = GetComponent<Collider>(true);
    col->bounciness = 0.0f;
    col->layer = Layer::Player;

    // プレイヤーのレイヤーを設定.
    gameObject->setLayer(Layer::Player);

    // 足元トリガーを作成.
    createGroundCheck();
}

void Player::Update()
{
    const float moveSpeed = 4;

    // �������
    Vector3 cont;
    if (Input::GetKey(Keyboard::A))
    {
        cont.x = -1.0f;
    }
    else if (Input::GetKey(Keyboard::D))
    {
        cont.x = 1.0f;
    }
    else
    {
        cont.x = 0.0f;
    }

    if (Input::GetKey(Keyboard::S))
    {
        cont.z = -1.0f;
    }
    else if (Input::GetKey(Keyboard::W))
    {
        cont.z = 1.0f;
    }
    else
    {
        cont.z = 0.0f;
    }

    // ジャンプ中は重力を徐々に強くする.
    if (jumpTime > 0 && jumpTime <= 150)
    {
        // progress: 0.0 → 1.0（ジャンプ開始から終了まで）.
        float progress = static_cast<float>(jumpTime) / 150.0f;
        // sin曲線で重力を徐々に強く（0 → 30）.
        rb->gravityScale = 30.0f * std::sin(progress * 3.14159f * 0.5f);
    }
    else
    {
        rb->gravityScale = isGrounded() ? 0 : 30.0f;
    }

    if ((jumpTime == 0 && isGrounded()) || jumpTime != 0)
    {
        // ジャンプ入力時間を更新.
        jumpTime = Input::GetKey(Keyboard::Space) ? jumpTime + 1 : 0;
    }
    Vector3 Yvelocity = Vector3(0, 0, 0);

    // 接地中かつジャンプ入力があればジャンプ.
    if (jumpTime > 0 && jumpTime <= 150) {
        // progress: 0.0 → 1.0（ジャンプ開始から終了まで）.
        float progress = static_cast<float>(jumpTime) / 150.0f;
        // cos曲線で上昇力を最初は強く、最後は弱く（10 → 0）.
        float y = 10.0f * std::cos(progress * 3.14159f * 0.5f);
        Yvelocity = Vector3(0, y, 0);
    }

    cont = cont.normalized();

    // �J�����������l�����đ��x�x�N�g�����v�Z
    Vector3 camF = Camera::main->transform->forward;
    float camAngle = std::atan2(camF.x, camF.z) * UniDx::Rad2Deg;
    Vector3 velocity = (cont.normalized() * moveSpeed) * Quaternion::AngleAxis(camAngle, Vector3::up);
    float vAngle = std::atan2(velocity.x, velocity.z) * UniDx::Rad2Deg;

    rb->linearVelocity = velocity + Yvelocity;
    if (cont != Vector3::zero)
    {
        rb->rotation = Quaternion::Euler(0, vAngle, 0);
    }

    // �A�j���i���Ή��j
    animFrame += cont.magnitude();
}


void Player::OnTriggerEnter(Collider* other)
{
}


void Player::OnTriggerStay(Collider* other)
{
}


void Player::OnTriggerExit(Collider* other)
{
}


void Player::OnCollisionEnter(const Collision& collision)
{
    if (collision.collider->name == StringId::intern("Coin"))
    {
        MainGame::getInstance()->AddScore(1);
        Destroy(collision.collider->gameObject);
    }
}


void Player::OnCollisionStay(const Collision& collision)
{
}

void Player::OnCollisionExit(const Collision& collision)
{
}


void Player::onGroundEnter()
{
    groundContactCount++;
}


void Player::onGroundExit()
{
    groundContactCount--;
    if (groundContactCount < 0) groundContactCount = 0;  // 安全対策.
}


void Player::createGroundCheck()
{
    // 足元トリガー用の子GameObjectを作成.
    auto groundCheckObj = std::make_unique<GameObject>(u8"FootTrigger");

    // トリガーコライダーを追加（足元に配置、小さめのサイズ）.
    auto collider = groundCheckObj->AddComponent<AABBCollider>();
    collider->center = Vector3(0, 0, 0);
    collider->size = Vector3(0.2f, 0.1f, 0.2f);  // 足元の小さなボックス.
    collider->isTrigger = true;
    collider->layer = Layer::Player;
    // Groundレイヤーとのみ衝突.
    collider->layerMask = LayerMask::GetMask(Layer::Ground);

    // GroundCheckコンポーネントを追加.
    auto groundCheck = groundCheckObj->AddComponent<GroundCheck>();
    groundCheck->player = this;

    // 足元の位置に配置（プレイヤーの下端）.
    groundCheckObj->transform->localPosition = Vector3(0, -0.15f, 0);

    // 子オブジェクトとして追加.
    Transform::SetParent(std::move(groundCheckObj), transform);
}

