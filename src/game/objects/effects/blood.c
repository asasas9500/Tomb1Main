#include "game/objects/effects/blood.h"

#include "3dsystem/phd_math.h"
#include "game/items.h"
#include "global/vars.h"

void Blood_Setup(OBJECT_INFO *obj)
{
    obj->control = Blood_Control;
}

void Blood_Control(int16_t fx_num)
{
    FX_INFO *fx = &g_Effects[fx_num];
    fx->pos.x += (phd_sin(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->pos.z += (phd_cos(fx->pos.y_rot) * fx->speed) >> W2V_SHIFT;
    fx->counter++;
    if (fx->counter == 4) {
        fx->counter = 0;
        fx->frame_number--;
        if (fx->frame_number <= g_Objects[fx->object_number].nmeshes) {
            KillEffect(fx_num);
        }
    }
}
