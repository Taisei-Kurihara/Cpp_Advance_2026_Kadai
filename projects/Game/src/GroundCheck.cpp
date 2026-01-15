#include "GroundCheck.h"
#include "Player.h"
#include <UniDx/Layer.h>

using namespace UniDx;

void GroundCheck::OnTriggerEnter(Collider* other)
{
    // Groundレイヤーのコライダーに接触.
    if(other->layer == Layer::Ground && player != nullptr)
    {
        player->onGroundEnter();
    }
}

void GroundCheck::OnTriggerExit(Collider* other)
{
    // Groundレイヤーのコライダーから離脱.
    if(other->layer == Layer::Ground && player != nullptr)
    {
        player->onGroundExit();
    }
}
