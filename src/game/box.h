#ifndef TOMB1MAIN_GAME_BOX_H
#define TOMB1MAIN_GAME_BOX_H

#include "game/types.h"

// clang-format off
#define InitialiseCreature      ((void          __cdecl(*)(int16_t item_num))0x0040DA60)
#define CalculateTarget         ((int32_t       __cdecl(*)(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT))0x0040E850)
// clang-format on

#endif
