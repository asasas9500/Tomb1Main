#include "game/effect_routines/finish_level.h"

#include "global/vars.h"

void FX_FinishLevel(ITEM_INFO *item)
{
    g_LevelComplete = true;
}
