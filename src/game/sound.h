#ifndef T1M_GAME_SOUND_H
#define T1M_GAME_SOUND_H

#include "global/types.h"

#include <stdbool.h>
#include <stdint.h>

bool Sound_Effect(int32_t sfx_num, PHD_3DPOS *pos, uint32_t flags);
bool Sound_StopEffect(int32_t sfx_num, PHD_3DPOS *pos);
void Sound_UpdateEffects();
void Sound_ResetEffects();
void Sound_ResetAmbientLoudness();
void Sound_StopAmbientSounds();
void Sound_AdjustMasterVolume(int8_t volume);

#endif
