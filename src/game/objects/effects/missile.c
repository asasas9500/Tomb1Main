#include "game/objects/effects/missile.h"

#include "3dsystem/phd_math.h"
#include "game/control.h"
#include "game/draw.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/random.h"
#include "game/room.h"
#include "game/sound.h"
#include "global/vars.h"

#define SHARD_DAMAGE 30
#define ROCKET_DAMAGE 100
#define ROCKET_RANGE SQUARE(WALL_L) // = 1048576

void Missile_Setup(OBJECT_INFO *obj)
{
    obj->control = Missile_Control;
}

void Missile_Control(int16_t fx_num)
{
    FX_INFO *fx = &g_Effects[fx_num];

    int32_t speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.y += (fx->speed * phd_sin(-fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.z += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.x += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;

    int16_t room_num = fx->room_number;
    FLOOR_INFO *floor =
        Room_GetFloor(fx->pos.x, fx->pos.y, fx->pos.z, &room_num);
    int32_t height = Room_GetHeight(floor, fx->pos.x, fx->pos.y, fx->pos.z);
    int32_t ceiling = Room_GetCeiling(floor, fx->pos.x, fx->pos.y, fx->pos.z);

    if (fx->pos.y >= height || fx->pos.y <= ceiling) {
        if (fx->object_number == O_MISSILE2) {
            fx->object_number = O_RICOCHET1;
            fx->frame_number = -Random_GetControl() / 11000;
            fx->speed = 0;
            fx->counter = 6;
            Sound_Effect(SFX_LARA_RICOCHET, &fx->pos, SPM_NORMAL);
        } else {
            fx->object_number = O_EXPLOSION1;
            fx->frame_number = 0;
            fx->speed = 0;
            fx->counter = 0;
            Sound_Effect(SFX_ATLANTEAN_EXPLODE, &fx->pos, SPM_NORMAL);

            int32_t x = fx->pos.x - g_LaraItem->pos.x;
            int32_t y = fx->pos.y - g_LaraItem->pos.y;
            int32_t z = fx->pos.z - g_LaraItem->pos.z;
            int32_t range = SQUARE(x) + SQUARE(y) + SQUARE(z);
            if (range < ROCKET_RANGE) {
                g_LaraItem->hit_points -=
                    (int16_t)(ROCKET_DAMAGE * (ROCKET_RANGE - range) / ROCKET_RANGE);
                g_LaraItem->hit_status = 1;
            }
        }
        return;
    }

    if (room_num != fx->room_number) {
        EffectNewRoom(fx_num, room_num);
    }

    if (!Lara_IsNearItem(&fx->pos, 200)) {
        return;
    }

    if (fx->object_number == O_MISSILE2) {
        g_LaraItem->hit_points -= SHARD_DAMAGE;
        fx->object_number = O_BLOOD1;
        Sound_Effect(SFX_LARA_BULLETHIT, &fx->pos, SPM_NORMAL);
    } else {
        g_LaraItem->hit_points -= ROCKET_DAMAGE;
        fx->object_number = O_EXPLOSION1;
        if (g_LaraItem->hit_points > 0) {
            Sound_Effect(SFX_LARA_INJURY, &g_LaraItem->pos, SPM_NORMAL);
            g_Lara.spaz_effect = fx;
            g_Lara.spaz_effect_count = 5;
        }
        Sound_Effect(SFX_ATLANTEAN_EXPLODE, &fx->pos, SPM_NORMAL);
    }
    g_LaraItem->hit_status = 1;

    fx->frame_number = 0;
    fx->pos.y_rot = g_LaraItem->pos.y_rot;
    fx->speed = g_LaraItem->speed;
    fx->counter = 0;
}
