#pragma once

// Public gun routines.

#include "global/types.h"

void Gun_Control(void);
void Gun_InitialiseNewWeapon(void);
void Gun_AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
int32_t Gun_FireWeapon(
    int32_t weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles);
void Gun_HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int32_t damage);
