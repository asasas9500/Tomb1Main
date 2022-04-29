#include "game/gun/gun_pistols.h"

#include "game/gun/gun_misc.h"
#include "game/input.h"
#include "game/sound.h"
#include "global/vars.h"

void Gun_Pistols_Draw(LARA_GUN_TYPE weapon_type)
{
    int16_t ani = g_Lara.left_arm.frame_number;
    ani++;

    if (ani < AF_G_DRAW1 || ani > AF_G_DRAW2_L) {
        ani = AF_G_DRAW1;
    } else if (ani == AF_G_DRAW2) {
        Gun_Pistols_DrawMeshes(weapon_type);
        Sound_Effect(SFX_LARA_DRAW, &g_LaraItem->pos, SPM_NORMAL);
    } else if (ani == AF_G_DRAW2_L) {
        Gun_Pistols_Ready();
        ani = AF_G_AIM;
    }

    g_Lara.left_arm.frame_number = ani;
    g_Lara.right_arm.frame_number = ani;
}

void Gun_Pistols_Undraw(LARA_GUN_TYPE weapon_type)
{
    int16_t anil = g_Lara.left_arm.frame_number;
    if (anil >= AF_G_RECOIL) {
        anil = AF_G_AIM_L;
    } else if (anil > AF_G_AIM && anil < AF_G_DRAW1) {
        g_Lara.left_arm.x_rot -= g_Lara.left_arm.x_rot / anil;
        g_Lara.left_arm.y_rot -= g_Lara.left_arm.y_rot / anil;
        anil--;
    } else if (anil == AF_G_AIM) {
        g_Lara.left_arm.x_rot = 0;
        g_Lara.left_arm.y_rot = 0;
        g_Lara.left_arm.z_rot = 0;
        anil = AF_G_DRAW2_L;
    } else if (anil > AF_G_DRAW1) {
        anil--;
        if (anil == AF_G_DRAW2) {
            Gun_Pistols_UndrawMeshLeft(weapon_type);
        }
    }
    g_Lara.left_arm.frame_number = anil;

    int16_t anir = g_Lara.right_arm.frame_number;
    if (anir >= AF_G_RECOIL) {
        anir = AF_G_AIM_L;
    } else if (anir > AF_G_AIM && anir < AF_G_DRAW1) {
        g_Lara.right_arm.x_rot -= g_Lara.right_arm.x_rot / anir;
        g_Lara.right_arm.y_rot -= g_Lara.right_arm.y_rot / anir;
        anir--;
    } else if (anir == AF_G_AIM) {
        g_Lara.right_arm.x_rot = 0;
        g_Lara.right_arm.y_rot = 0;
        g_Lara.right_arm.z_rot = 0;
        anir = AF_G_DRAW2_L;
    } else if (anir > AF_G_DRAW1) {
        anir--;
        if (anir == AF_G_DRAW2) {
            Gun_Pistols_UndrawMeshRight(weapon_type);
        }
    }
    g_Lara.right_arm.frame_number = anir;

    if (anil == AF_G_DRAW1 && anir == AF_G_DRAW1) {
        g_Lara.left_arm.lock = 0;
        g_Lara.right_arm.lock = 0;
        g_Lara.left_arm.frame_number = AF_G_AIM;
        g_Lara.right_arm.frame_number = AF_G_AIM;
        g_Lara.gun_status = LGS_ARMLESS;
        g_Lara.target = NULL;
    }

    g_Lara.head_x_rot = (g_Lara.right_arm.x_rot + g_Lara.left_arm.x_rot) / 4;
    g_Lara.head_y_rot = (g_Lara.right_arm.y_rot + g_Lara.left_arm.y_rot) / 4;
    g_Lara.torso_x_rot = (g_Lara.right_arm.x_rot + g_Lara.left_arm.x_rot) / 4;
    g_Lara.torso_y_rot = (g_Lara.right_arm.y_rot + g_Lara.left_arm.y_rot) / 4;
}

void Gun_Pistols_Ready(void)
{
    g_Lara.gun_status = LGS_READY;
    g_Lara.left_arm.x_rot = 0;
    g_Lara.left_arm.y_rot = 0;
    g_Lara.left_arm.z_rot = 0;
    g_Lara.left_arm.lock = 0;
    g_Lara.right_arm.x_rot = 0;
    g_Lara.right_arm.y_rot = 0;
    g_Lara.right_arm.z_rot = 0;
    g_Lara.right_arm.lock = 0;
    g_Lara.head_x_rot = 0;
    g_Lara.head_y_rot = 0;
    g_Lara.torso_x_rot = 0;
    g_Lara.torso_y_rot = 0;
    g_Lara.target = NULL;
    g_Lara.right_arm.frame_base = g_Objects[O_PISTOLS].frame_base;
    g_Lara.left_arm.frame_base = g_Objects[O_PISTOLS].frame_base;
}

void Gun_Pistols_DrawMeshes(LARA_GUN_TYPE weapon_type)
{
    int16_t object_num = O_PISTOLS;
    if (weapon_type == LGT_MAGNUMS) {
        object_num = O_MAGNUM;
    } else if (weapon_type == LGT_UZIS) {
        object_num = O_UZI;
    }

    g_Lara.mesh_ptrs[LM_HAND_L] =
        g_Meshes[g_Objects[object_num].mesh_index + LM_HAND_L];
    g_Lara.mesh_ptrs[LM_HAND_R] =
        g_Meshes[g_Objects[object_num].mesh_index + LM_HAND_R];
    g_Lara.mesh_ptrs[LM_THIGH_L] =
        g_Meshes[g_Objects[O_LARA].mesh_index + LM_THIGH_L];
    g_Lara.mesh_ptrs[LM_THIGH_R] =
        g_Meshes[g_Objects[O_LARA].mesh_index + LM_THIGH_R];
}

void Gun_Pistols_UndrawMeshLeft(LARA_GUN_TYPE weapon_type)
{
    int16_t object_num = O_PISTOLS;
    if (weapon_type == LGT_MAGNUMS) {
        object_num = O_MAGNUM;
    } else if (weapon_type == LGT_UZIS) {
        object_num = O_UZI;
    }
    g_Lara.mesh_ptrs[LM_THIGH_L] =
        g_Meshes[g_Objects[object_num].mesh_index + LM_THIGH_L];
    g_Lara.mesh_ptrs[LM_HAND_L] =
        g_Meshes[g_Objects[O_LARA].mesh_index + LM_HAND_L];
    Sound_Effect(SFX_LARA_HOLSTER, &g_LaraItem->pos, SPM_NORMAL);
}

void Gun_Pistols_UndrawMeshRight(LARA_GUN_TYPE weapon_type)
{
    int16_t object_num = O_PISTOLS;
    if (weapon_type == LGT_MAGNUMS) {
        object_num = O_MAGNUM;
    } else if (weapon_type == LGT_UZIS) {
        object_num = O_UZI;
    }
    g_Lara.mesh_ptrs[LM_THIGH_R] =
        g_Meshes[g_Objects[object_num].mesh_index + LM_THIGH_R];
    g_Lara.mesh_ptrs[LM_HAND_R] =
        g_Meshes[g_Objects[O_LARA].mesh_index + LM_HAND_R];
    Sound_Effect(SFX_LARA_HOLSTER, &g_LaraItem->pos, SPM_NORMAL);
}

void Gun_Pistols_Control(LARA_GUN_TYPE weapon_type)
{
    WEAPON_INFO *winfo = &g_Weapons[weapon_type];

    if (g_Input.action) {
        Gun_TargetInfo(winfo);
    } else {
        g_Lara.target = NULL;
    }
    if (!g_Lara.target) {
        Gun_GetNewTarget(winfo);
    }

    Gun_AimWeapon(winfo, &g_Lara.left_arm);
    Gun_AimWeapon(winfo, &g_Lara.right_arm);

    if (g_Lara.left_arm.lock && !g_Lara.right_arm.lock) {
        g_Lara.head_x_rot = g_Lara.left_arm.x_rot / 2;
        g_Lara.head_y_rot = g_Lara.left_arm.y_rot / 2;
        g_Lara.torso_x_rot = g_Lara.left_arm.x_rot / 2;
        g_Lara.torso_y_rot = g_Lara.left_arm.y_rot / 2;
    } else if (!g_Lara.left_arm.lock && g_Lara.right_arm.lock) {
        g_Lara.head_x_rot = g_Lara.right_arm.x_rot / 2;
        g_Lara.head_y_rot = g_Lara.right_arm.y_rot / 2;
        g_Lara.torso_x_rot = g_Lara.right_arm.x_rot / 2;
        g_Lara.torso_y_rot = g_Lara.right_arm.y_rot / 2;
    } else if (g_Lara.left_arm.lock && g_Lara.right_arm.lock) {
        g_Lara.head_x_rot =
            (g_Lara.right_arm.x_rot + g_Lara.left_arm.x_rot) / 4;
        g_Lara.head_y_rot =
            (g_Lara.right_arm.y_rot + g_Lara.left_arm.y_rot) / 4;
        g_Lara.torso_x_rot =
            (g_Lara.right_arm.x_rot + g_Lara.left_arm.x_rot) / 4;
        g_Lara.torso_y_rot =
            (g_Lara.right_arm.y_rot + g_Lara.left_arm.y_rot) / 4;
    }

    Gun_Pistols_Animate(weapon_type);
}

void Gun_Pistols_Animate(LARA_GUN_TYPE weapon_type)
{
    PHD_ANGLE angles[2];
    WEAPON_INFO *winfo = &g_Weapons[weapon_type];

    int16_t anir = g_Lara.right_arm.frame_number;
    if (g_Lara.right_arm.lock || (g_Input.action && !g_Lara.target)) {
        if (anir >= AF_G_AIM && anir < AF_G_AIM_L) {
            anir++;
        } else if (anir == AF_G_AIM_L && g_Input.action) {
            angles[0] = g_Lara.right_arm.y_rot + g_LaraItem->pos.y_rot;
            angles[1] = g_Lara.right_arm.x_rot;
            if (Gun_FireWeapon(
                    weapon_type, g_Lara.target, g_LaraItem, angles)) {
                g_Lara.right_arm.flash_gun = winfo->flash_time;
                Sound_Effect(winfo->sample_num, &g_LaraItem->pos, SPM_NORMAL);
            }
            anir = AF_G_RECOIL;
        } else if (anir >= AF_G_RECOIL) {
            anir++;
            if (anir == AF_G_RECOIL + winfo->recoil_frame) {
                anir = AF_G_AIM_L;
            }
        }
    } else if (anir >= AF_G_RECOIL) {
        anir = AF_G_AIM_L;
    } else if (anir > AF_G_AIM && anir <= AF_G_AIM_L) {
        anir--;
    }
    g_Lara.right_arm.frame_number = anir;

    int16_t anil = g_Lara.left_arm.frame_number;
    if (g_Lara.left_arm.lock || (g_Input.action && !g_Lara.target)) {
        if (anil >= AF_G_AIM && anil < AF_G_AIM_L) {
            anil++;
        } else if (anil == AF_G_AIM_L && g_Input.action) {
            angles[0] = g_Lara.left_arm.y_rot + g_LaraItem->pos.y_rot;
            angles[1] = g_Lara.left_arm.x_rot;
            if (Gun_FireWeapon(
                    weapon_type, g_Lara.target, g_LaraItem, angles)) {
                g_Lara.left_arm.flash_gun = winfo->flash_time;
                Sound_Effect(winfo->sample_num, &g_LaraItem->pos, SPM_NORMAL);
            }
            anil = AF_G_RECOIL;
        } else if (anil >= AF_G_RECOIL) {
            anil++;
            if (anil == AF_G_RECOIL + winfo->recoil_frame) {
                anil = AF_G_AIM_L;
            }
        }
    } else if (anil >= AF_G_RECOIL) {
        anil = AF_G_AIM_L;
    } else if (anil > AF_G_AIM && anil <= AF_G_AIM_L) {
        anil--;
    }
    g_Lara.left_arm.frame_number = anil;
}
