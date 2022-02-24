#define SAVEGAME_IMPL
#include "game/savegame_legacy.h"

#include "game/control.h"
#include "game/gameflow.h"
#include "game/inv.h"
#include "game/items.h"
#include "game/lara.h"
#include "game/lot.h"
#include "game/shell.h"
#include "global/vars.h"
#include "log.h"
#include "memory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SAVE_CREATURE (1 << 7)
#define SAVEGAME_LEGACY_TITLE_SIZE 75
#define SAVEGAME_LEGACY_MAX_BUFFER_SIZE (20 * 1024)

typedef struct SAVEGAME_LEGACY_ITEM_STATS {
    uint8_t num_pickup1;
    uint8_t num_pickup2;
    uint8_t num_puzzle1;
    uint8_t num_puzzle2;
    uint8_t num_puzzle3;
    uint8_t num_puzzle4;
    uint8_t num_key1;
    uint8_t num_key2;
    uint8_t num_key3;
    uint8_t num_key4;
    uint8_t num_leadbar;
    uint8_t dummy;
} SAVEGAME_LEGACY_ITEM_STATS;

static int m_SGBufPos = 0;
static char *m_SGBufPtr = NULL;

static bool Savegame_Legacy_NeedsEvilLaraFix();

static void Savegame_Legacy_Reset(char *buffer);
static void Savegame_Legacy_Skip(int size);

static void Savegame_Legacy_Read(void *pointer, int size);
static void Savegame_Legacy_ReadArm(LARA_ARM *arm);
static void Savegame_Legacy_ReadLara(LARA_INFO *lara);
static void Savegame_Legacy_ReadLOT(LOT_INFO *lot);

static void Savegame_Legacy_Write(void *pointer, int size);
static void Savegame_Legacy_WriteArm(LARA_ARM *arm);
static void Savegame_Legacy_WriteLara(LARA_INFO *lara);
static void Savegame_Legacy_WriteLOT(LOT_INFO *lot);

static bool Savegame_Legacy_NeedsEvilLaraFix(char *buffer)
{
    // Heuristic for issue #261.
    // Tomb1Main enables save_flags for Evil Lara, but OG TombATI does not. As
    // a consequence, Atlantis saves made with OG TombATI (which includes the
    // ones available for download on Stella's website) have different layout
    // than the saves made with Tomb1Main. This was discovered after it was too
    // late to make a backwards incompatible change. At the same time, enabling
    // save_flags for Evil Lara is desirable, as not doing this causes her to
    // freeze when the player reloads a save made in her room. This function is
    // used to determine whether the save about to be loaded includes
    // save_flags for Evil Lara or not. Since savegames only contain very
    // concise information, we must make an educated guess here.

    assert(buffer);

    bool result = false;
    if (g_CurrentLevel != 14) {
        return result;
    }

    Savegame_Legacy_Reset(buffer);
    Savegame_Legacy_Skip(SAVEGAME_LEGACY_TITLE_SIZE); // level title
    Savegame_Legacy_Skip(sizeof(int32_t)); // save counter
    for (int i = 0; i < g_GameFlow.level_count; i++) {
        Savegame_Legacy_Skip(sizeof(uint16_t)); // pistol ammo
        Savegame_Legacy_Skip(sizeof(uint16_t)); // magnum ammo
        Savegame_Legacy_Skip(sizeof(uint16_t)); // uzi ammo
        Savegame_Legacy_Skip(sizeof(uint16_t)); // shotgun ammo
        Savegame_Legacy_Skip(sizeof(uint8_t)); // small medis
        Savegame_Legacy_Skip(sizeof(uint8_t)); // big medis
        Savegame_Legacy_Skip(sizeof(uint8_t)); // scions
        Savegame_Legacy_Skip(sizeof(int8_t)); // gun status
        Savegame_Legacy_Skip(sizeof(int8_t)); // gun type
        Savegame_Legacy_Skip(sizeof(uint16_t)); // flags
    }
    Savegame_Legacy_Skip(sizeof(uint32_t)); // timer
    Savegame_Legacy_Skip(sizeof(uint32_t)); // kills
    Savegame_Legacy_Skip(sizeof(uint16_t)); // secrets
    Savegame_Legacy_Skip(sizeof(uint16_t)); // current level
    Savegame_Legacy_Skip(sizeof(uint8_t)); // pickups
    Savegame_Legacy_Skip(sizeof(uint8_t)); // bonus_flag
    Savegame_Legacy_Skip(sizeof(SAVEGAME_LEGACY_ITEM_STATS)); // item stats
    Savegame_Legacy_Skip(sizeof(int32_t)); // flipmap status
    Savegame_Legacy_Skip(MAX_FLIP_MAPS * sizeof(int8_t)); // flipmap table
    Savegame_Legacy_Skip(g_NumberCameras * sizeof(int16_t)); // cameras

    for (int i = 0; i < g_LevelItemCount; i++) {
        ITEM_INFO *item = &g_Items[i];
        OBJECT_INFO *obj = &g_Objects[item->object_number];

        ITEM_INFO tmp_item;

        if (obj->save_position) {
            Savegame_Legacy_Read(&tmp_item.pos, sizeof(PHD_3DPOS));
            Savegame_Legacy_Skip(sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.speed, sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.fall_speed, sizeof(int16_t));
        }
        if (obj->save_anim) {
            Savegame_Legacy_Read(&tmp_item.current_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.goal_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(
                &tmp_item.required_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.anim_number, sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.frame_number, sizeof(int16_t));
        }
        if (obj->save_hitpoints) {
            Savegame_Legacy_Read(&tmp_item.hit_points, sizeof(int16_t));
        }
        if (obj->save_flags) {
            Savegame_Legacy_Read(&tmp_item.flags, sizeof(int16_t));
            Savegame_Legacy_Read(&tmp_item.timer, sizeof(int16_t));
            if (tmp_item.flags & SAVE_CREATURE) {
                CREATURE_INFO tmp_creature;
                Savegame_Legacy_Read(
                    &tmp_creature.head_rotation, sizeof(int16_t));
                Savegame_Legacy_Read(
                    &tmp_creature.neck_rotation, sizeof(int16_t));
                Savegame_Legacy_Read(
                    &tmp_creature.maximum_turn, sizeof(int16_t));
                Savegame_Legacy_Read(&tmp_creature.flags, sizeof(int16_t));
                Savegame_Legacy_Read(&tmp_creature.mood, sizeof(int32_t));
            }
        }

        // check for exceptionally high item positions.
        if ((ABS(tmp_item.pos.x) | ABS(tmp_item.pos.y) | ABS(tmp_item.pos.z))
            & 0xFF000000) {
            result = true;
        }
    }

    return result;
}

static void Savegame_Legacy_Reset(char *buffer)
{
    m_SGBufPos = 0;
    m_SGBufPtr = buffer;
}

static void Savegame_Legacy_Skip(int size)
{
    m_SGBufPtr += size;
    m_SGBufPos += size; // missing from OG
}

static void Savegame_Legacy_Write(void *pointer, int size)
{
    m_SGBufPos += size;
    if (m_SGBufPos >= SAVEGAME_LEGACY_MAX_BUFFER_SIZE) {
        Shell_ExitSystem("FATAL: Savegame is too big to fit in buffer");
    }

    char *data = (char *)pointer;
    for (int i = 0; i < size; i++) {
        *m_SGBufPtr++ = *data++;
    }
}

static void Savegame_Legacy_WriteLara(LARA_INFO *lara)
{
    int32_t tmp32 = 0;

    Savegame_Legacy_Write(&lara->item_number, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->gun_status, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->gun_type, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->request_gun_type, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->calc_fall_speed, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->water_status, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->pose_count, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->hit_frame, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->hit_direction, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->air, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->dive_timer, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->death_timer, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->current_active, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->spaz_effect_count, sizeof(int16_t));

    // OG just writes the pointer address (!)
    if (lara->spaz_effect) {
        tmp32 = (size_t)lara->spaz_effect - (size_t)g_Effects;
    }
    Savegame_Legacy_Write(&tmp32, sizeof(int32_t));

    Savegame_Legacy_Write(&lara->mesh_effects, sizeof(int32_t));

    for (int i = 0; i < LM_NUMBER_OF; i++) {
        tmp32 = (size_t)lara->mesh_ptrs[i] - (size_t)g_MeshBase;
        Savegame_Legacy_Write(&tmp32, sizeof(int32_t));
    }

    // OG just writes the pointer address (!) assuming it's a non-existing mesh
    // 16 (!!) which happens to be g_Lara's current target. Just write NULL.
    tmp32 = 0;
    Savegame_Legacy_Write(&tmp32, sizeof(int32_t));

    Savegame_Legacy_Write(&lara->target_angles[0], sizeof(PHD_ANGLE));
    Savegame_Legacy_Write(&lara->target_angles[1], sizeof(PHD_ANGLE));
    Savegame_Legacy_Write(&lara->turn_rate, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->move_angle, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->head_y_rot, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->head_x_rot, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->head_z_rot, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->torso_y_rot, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->torso_x_rot, sizeof(int16_t));
    Savegame_Legacy_Write(&lara->torso_z_rot, sizeof(int16_t));

    Savegame_Legacy_WriteArm(&lara->left_arm);
    Savegame_Legacy_WriteArm(&lara->right_arm);
    Savegame_Legacy_Write(&lara->pistols, sizeof(AMMO_INFO));
    Savegame_Legacy_Write(&lara->magnums, sizeof(AMMO_INFO));
    Savegame_Legacy_Write(&lara->uzis, sizeof(AMMO_INFO));
    Savegame_Legacy_Write(&lara->shotgun, sizeof(AMMO_INFO));
    Savegame_Legacy_WriteLOT(&lara->LOT);
}

static void Savegame_Legacy_WriteArm(LARA_ARM *arm)
{
    int32_t frame_base = (size_t)arm->frame_base - (size_t)g_AnimFrames;
    Savegame_Legacy_Write(&frame_base, sizeof(int32_t));
    Savegame_Legacy_Write(&arm->frame_number, sizeof(int16_t));
    Savegame_Legacy_Write(&arm->lock, sizeof(int16_t));
    Savegame_Legacy_Write(&arm->y_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Write(&arm->x_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Write(&arm->z_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Write(&arm->flash_gun, sizeof(int16_t));
}

static void Savegame_Legacy_WriteLOT(LOT_INFO *lot)
{
    // it casually saves a pointer again!
    Savegame_Legacy_Write(&lot->node, sizeof(int32_t));

    Savegame_Legacy_Write(&lot->head, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->tail, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->search_number, sizeof(uint16_t));
    Savegame_Legacy_Write(&lot->block_mask, sizeof(uint16_t));
    Savegame_Legacy_Write(&lot->step, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->drop, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->fly, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->zone_count, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->target_box, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->required_box, sizeof(int16_t));
    Savegame_Legacy_Write(&lot->target, sizeof(PHD_VECTOR));
}

static void Savegame_Legacy_Read(void *pointer, int size)
{
    m_SGBufPos += size;
    char *data = (char *)pointer;
    for (int i = 0; i < size; i++)
        *data++ = *m_SGBufPtr++;
}

static void Savegame_Legacy_ReadLara(LARA_INFO *lara)
{
    int32_t tmp32 = 0;

    Savegame_Legacy_Read(&lara->item_number, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->gun_status, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->gun_type, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->request_gun_type, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->calc_fall_speed, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->water_status, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->pose_count, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->hit_frame, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->hit_direction, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->air, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->dive_timer, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->death_timer, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->current_active, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->spaz_effect_count, sizeof(int16_t));

    lara->spaz_effect = NULL;
    Savegame_Legacy_Skip(sizeof(FX_INFO *));

    Savegame_Legacy_Read(&lara->mesh_effects, sizeof(int32_t));
    for (int i = 0; i < LM_NUMBER_OF; i++) {
        Savegame_Legacy_Read(&tmp32, sizeof(int32_t));
        lara->mesh_ptrs[i] = (int16_t *)((size_t)g_MeshBase + (size_t)tmp32);
    }

    lara->target = NULL;
    Savegame_Legacy_Skip(sizeof(ITEM_INFO *));

    Savegame_Legacy_Read(&lara->target_angles[0], sizeof(PHD_ANGLE));
    Savegame_Legacy_Read(&lara->target_angles[1], sizeof(PHD_ANGLE));
    Savegame_Legacy_Read(&lara->turn_rate, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->move_angle, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->head_y_rot, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->head_x_rot, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->head_z_rot, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->torso_y_rot, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->torso_x_rot, sizeof(int16_t));
    Savegame_Legacy_Read(&lara->torso_z_rot, sizeof(int16_t));

    Savegame_Legacy_ReadArm(&lara->left_arm);
    Savegame_Legacy_ReadArm(&lara->right_arm);
    Savegame_Legacy_Read(&lara->pistols, sizeof(AMMO_INFO));
    Savegame_Legacy_Read(&lara->magnums, sizeof(AMMO_INFO));
    Savegame_Legacy_Read(&lara->uzis, sizeof(AMMO_INFO));
    Savegame_Legacy_Read(&lara->shotgun, sizeof(AMMO_INFO));
    Savegame_Legacy_ReadLOT(&lara->LOT);
}

static void Savegame_Legacy_ReadArm(LARA_ARM *arm)
{
    int32_t frame_base;
    Savegame_Legacy_Read(&frame_base, sizeof(int32_t));
    arm->frame_base = (int16_t *)((size_t)g_AnimFrames + (size_t)frame_base);

    Savegame_Legacy_Read(&arm->frame_number, sizeof(int16_t));
    Savegame_Legacy_Read(&arm->lock, sizeof(int16_t));
    Savegame_Legacy_Read(&arm->y_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Read(&arm->x_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Read(&arm->z_rot, sizeof(PHD_ANGLE));
    Savegame_Legacy_Read(&arm->flash_gun, sizeof(int16_t));
}

static void Savegame_Legacy_ReadLOT(LOT_INFO *lot)
{
    lot->node = NULL;
    Savegame_Legacy_Skip(sizeof(BOX_NODE *));

    Savegame_Legacy_Read(&lot->head, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->tail, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->search_number, sizeof(uint16_t));
    Savegame_Legacy_Read(&lot->block_mask, sizeof(uint16_t));
    Savegame_Legacy_Read(&lot->step, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->drop, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->fly, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->zone_count, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->target_box, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->required_box, sizeof(int16_t));
    Savegame_Legacy_Read(&lot->target, sizeof(PHD_VECTOR));
}

char *Savegame_Legacy_GetSaveFileName(int32_t slot)
{
    size_t out_size =
        snprintf(NULL, 0, g_GameFlow.savegame_fmt_legacy, slot) + 1;
    char *out = Memory_Alloc(out_size);
    snprintf(out, out_size, g_GameFlow.savegame_fmt_legacy, slot);
    return out;
}

bool Savegame_Legacy_FillInfo(MYFILE *fp, SAVEGAME_INFO *info)
{
    File_Seek(fp, 0, SEEK_SET);

    char title[SAVEGAME_LEGACY_TITLE_SIZE];
    File_Read(title, sizeof(char), SAVEGAME_LEGACY_TITLE_SIZE, fp);
    info->level_title = Memory_Dup(title);

    int32_t counter;
    File_Read(&counter, sizeof(int32_t), 1, fp);
    info->counter = counter;

    for (int i = 0; i < g_GameFlow.level_count; i++) {
        File_Skip(fp, sizeof(uint16_t)); // pistol ammo
        File_Skip(fp, sizeof(uint16_t)); // magnum ammo
        File_Skip(fp, sizeof(uint16_t)); // uzi ammo
        File_Skip(fp, sizeof(uint16_t)); // shotgun ammo
        File_Skip(fp, sizeof(uint8_t)); // small medis
        File_Skip(fp, sizeof(uint8_t)); // big medis
        File_Skip(fp, sizeof(uint8_t)); // scions
        File_Skip(fp, sizeof(int8_t)); // gun status
        File_Skip(fp, sizeof(int8_t)); // gun type
        File_Skip(fp, sizeof(uint16_t)); // flags
    }
    File_Skip(fp, sizeof(uint32_t)); // timer
    File_Skip(fp, sizeof(uint32_t)); // kills
    File_Skip(fp, sizeof(uint16_t)); // secrets

    uint16_t level_num;
    File_Read(&level_num, sizeof(int16_t), 1, fp);
    info->level_num = level_num;

    return true;
}

bool Savegame_Legacy_LoadFromFile(MYFILE *fp, GAME_INFO *game_info)
{
    assert(game_info);

    int8_t tmp8;
    int16_t tmp16;
    int32_t tmp32;

    char *buffer = Memory_Alloc(File_Size(fp));
    File_Seek(fp, 0, FILE_SEEK_SET);
    File_Read(buffer, sizeof(char), File_Size(fp), fp);

    bool skip_reading_evil_lara = Savegame_Legacy_NeedsEvilLaraFix(buffer);
    if (skip_reading_evil_lara) {
        LOG_INFO("Enabling Evil Lara savegame fix");
    }

    Savegame_Legacy_Reset(buffer);
    Savegame_Legacy_Skip(SAVEGAME_LEGACY_TITLE_SIZE); // level title
    Savegame_Legacy_Skip(sizeof(int32_t)); // save counter

    assert(game_info->start);
    for (int i = 0; i < g_GameFlow.level_count; i++) {
        START_INFO *start = &game_info->start[i];
        Savegame_Legacy_Read(&start->pistol_ammo, sizeof(uint16_t));
        Savegame_Legacy_Read(&start->magnum_ammo, sizeof(uint16_t));
        Savegame_Legacy_Read(&start->uzi_ammo, sizeof(uint16_t));
        Savegame_Legacy_Read(&start->shotgun_ammo, sizeof(uint16_t));
        Savegame_Legacy_Read(&start->num_medis, sizeof(uint8_t));
        Savegame_Legacy_Read(&start->num_big_medis, sizeof(uint8_t));
        Savegame_Legacy_Read(&start->num_scions, sizeof(uint8_t));
        Savegame_Legacy_Read(&start->gun_status, sizeof(int8_t));
        Savegame_Legacy_Read(&start->gun_type, sizeof(int8_t));
        uint16_t flags;
        Savegame_Legacy_Read(&flags, sizeof(uint16_t));
        start->flags.available = flags & 1 ? 1 : 0;
        start->flags.got_pistols = flags & 2 ? 1 : 0;
        start->flags.got_magnums = flags & 4 ? 1 : 0;
        start->flags.got_uzis = flags & 8 ? 1 : 0;
        start->flags.got_shotgun = flags & 16 ? 1 : 0;
        start->flags.costume = flags & 32 ? 1 : 0;
    }

    Savegame_Legacy_Read(&game_info->stats.timer, sizeof(uint32_t));
    Savegame_Legacy_Read(&game_info->stats.kill_count, sizeof(uint32_t));
    Savegame_Legacy_Read(&game_info->stats.secret_flags, sizeof(uint16_t));
    Savegame_Legacy_Read(&g_CurrentLevel, sizeof(uint16_t));
    Savegame_Legacy_Read(&game_info->stats.pickup_count, sizeof(uint8_t));
    Savegame_Legacy_Read(&game_info->bonus_flag, sizeof(uint8_t));

    Savegame_SetCurrentPosition(g_CurrentLevel);
    for (int i = 0; i < g_GameFlow.level_count; i++) {
        Savegame_ResetEndInfo(i);
    }
    game_info->death_counter_supported = false;
    game_info->end[g_CurrentLevel].stats = game_info->stats;

    InitialiseLaraInventory(g_CurrentLevel);
    SAVEGAME_LEGACY_ITEM_STATS item_stats = { 0 };
    Savegame_Legacy_Read(&item_stats, sizeof(item_stats));
    Inv_AddItemNTimes(O_PICKUP_ITEM1, item_stats.num_pickup1);
    Inv_AddItemNTimes(O_PICKUP_ITEM2, item_stats.num_pickup2);
    Inv_AddItemNTimes(O_PUZZLE_ITEM1, item_stats.num_puzzle1);
    Inv_AddItemNTimes(O_PUZZLE_ITEM2, item_stats.num_puzzle2);
    Inv_AddItemNTimes(O_PUZZLE_ITEM3, item_stats.num_puzzle3);
    Inv_AddItemNTimes(O_PUZZLE_ITEM4, item_stats.num_puzzle4);
    Inv_AddItemNTimes(O_KEY_ITEM1, item_stats.num_key1);
    Inv_AddItemNTimes(O_KEY_ITEM2, item_stats.num_key2);
    Inv_AddItemNTimes(O_KEY_ITEM3, item_stats.num_key3);
    Inv_AddItemNTimes(O_KEY_ITEM4, item_stats.num_key4);
    Inv_AddItemNTimes(O_LEADBAR_ITEM, item_stats.num_leadbar);

    Savegame_Legacy_Read(&tmp32, sizeof(int32_t));
    if (tmp32) {
        FlipMap();
    }

    for (int i = 0; i < MAX_FLIP_MAPS; i++) {
        Savegame_Legacy_Read(&tmp8, sizeof(int8_t));
        g_FlipMapTable[i] = tmp8 << 8;
    }

    for (int i = 0; i < g_NumberCameras; i++) {
        Savegame_Legacy_Read(&g_Camera.fixed[i].flags, sizeof(int16_t));
    }

    for (int i = 0; i < g_LevelItemCount; i++) {
        ITEM_INFO *item = &g_Items[i];
        OBJECT_INFO *obj = &g_Objects[item->object_number];

        if (obj->save_position) {
            Savegame_Legacy_Read(&item->pos, sizeof(PHD_3DPOS));
            Savegame_Legacy_Read(&tmp16, sizeof(int16_t));
            Savegame_Legacy_Read(&item->speed, sizeof(int16_t));
            Savegame_Legacy_Read(&item->fall_speed, sizeof(int16_t));

            if (item->room_number != tmp16) {
                ItemNewRoom(i, tmp16);
            }
        }

        if (obj->save_anim) {
            Savegame_Legacy_Read(&item->current_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(&item->goal_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(&item->required_anim_state, sizeof(int16_t));
            Savegame_Legacy_Read(&item->anim_number, sizeof(int16_t));
            Savegame_Legacy_Read(&item->frame_number, sizeof(int16_t));
        }

        if (obj->save_hitpoints) {
            Savegame_Legacy_Read(&item->hit_points, sizeof(int16_t));
        }

        if (obj->save_flags
            && (item->object_number != O_EVIL_LARA
                || !skip_reading_evil_lara)) {
            Savegame_Legacy_Read(&item->flags, sizeof(int16_t));
            Savegame_Legacy_Read(&item->timer, sizeof(int16_t));

            if (item->flags & IF_KILLED_ITEM) {
                KillItem(i);
                item->status = IS_DEACTIVATED;
            } else {
                if ((item->flags & 1) && !item->active) {
                    AddActiveItem(i);
                }
                item->status = (item->flags & 6) >> 1;
                if (item->flags & 8) {
                    item->gravity_status = 1;
                }
                if (!(item->flags & 16)) {
                    item->collidable = 0;
                }
            }

            if (item->flags & SAVE_CREATURE) {
                EnableBaddieAI(i, 1);
                CREATURE_INFO *creature = item->data;
                if (creature) {
                    Savegame_Legacy_Read(
                        &creature->head_rotation, sizeof(int16_t));
                    Savegame_Legacy_Read(
                        &creature->neck_rotation, sizeof(int16_t));
                    Savegame_Legacy_Read(
                        &creature->maximum_turn, sizeof(int16_t));
                    Savegame_Legacy_Read(&creature->flags, sizeof(int16_t));
                    Savegame_Legacy_Read(&creature->mood, sizeof(int32_t));
                } else {
                    Savegame_Legacy_Skip(4 * 2 + 4);
                }
            } else if (obj->intelligent) {
                item->data = NULL;
            }
        }
    }

    Savegame_Legacy_ReadLara(&g_Lara);
    Savegame_Legacy_Read(&g_FlipEffect, sizeof(int32_t));
    Savegame_Legacy_Read(&g_FlipTimer, sizeof(int32_t));
    Memory_FreePointer(&buffer);
    return true;
}

void Savegame_Legacy_SaveToFile(MYFILE *fp, GAME_INFO *game_info)
{
    assert(game_info);

    char *buffer = Memory_Alloc(SAVEGAME_LEGACY_MAX_BUFFER_SIZE);
    Savegame_Legacy_Reset(buffer);
    memset(m_SGBufPtr, 0, SAVEGAME_LEGACY_MAX_BUFFER_SIZE);

    char title[SAVEGAME_LEGACY_TITLE_SIZE];
    snprintf(
        title, SAVEGAME_LEGACY_TITLE_SIZE, "%s",
        g_GameFlow.levels[g_CurrentLevel].level_title);
    Savegame_Legacy_Write(title, SAVEGAME_LEGACY_TITLE_SIZE);
    Savegame_Legacy_Write(&g_SaveCounter, sizeof(int32_t));

    assert(game_info->start);
    for (int i = 0; i < g_GameFlow.level_count; i++) {
        START_INFO *start = &game_info->start[i];
        Savegame_Legacy_Write(&start->pistol_ammo, sizeof(uint16_t));
        Savegame_Legacy_Write(&start->magnum_ammo, sizeof(uint16_t));
        Savegame_Legacy_Write(&start->uzi_ammo, sizeof(uint16_t));
        Savegame_Legacy_Write(&start->shotgun_ammo, sizeof(uint16_t));
        Savegame_Legacy_Write(&start->num_medis, sizeof(uint8_t));
        Savegame_Legacy_Write(&start->num_big_medis, sizeof(uint8_t));
        Savegame_Legacy_Write(&start->num_scions, sizeof(uint8_t));
        Savegame_Legacy_Write(&start->gun_status, sizeof(int8_t));
        Savegame_Legacy_Write(&start->gun_type, sizeof(int8_t));
        uint16_t flags = 0;
        flags |= start->flags.available ? 1 : 0;
        flags |= start->flags.got_pistols ? 2 : 0;
        flags |= start->flags.got_magnums ? 4 : 0;
        flags |= start->flags.got_uzis ? 8 : 0;
        flags |= start->flags.got_shotgun ? 16 : 0;
        flags |= start->flags.costume ? 32 : 0;
        Savegame_Legacy_Write(&flags, sizeof(uint16_t));
    }

    Savegame_Legacy_Write(&game_info->stats.timer, sizeof(uint32_t));
    Savegame_Legacy_Write(&game_info->stats.kill_count, sizeof(uint32_t));
    Savegame_Legacy_Write(&game_info->stats.secret_flags, sizeof(uint16_t));
    Savegame_Legacy_Write(&g_CurrentLevel, sizeof(uint16_t));
    Savegame_Legacy_Write(&game_info->stats.pickup_count, sizeof(uint8_t));
    Savegame_Legacy_Write(&game_info->bonus_flag, sizeof(uint8_t));

    SAVEGAME_LEGACY_ITEM_STATS item_stats = {
        .num_pickup1 = Inv_RequestItem(O_PICKUP_ITEM1),
        .num_pickup2 = Inv_RequestItem(O_PICKUP_ITEM2),
        .num_puzzle1 = Inv_RequestItem(O_PUZZLE_ITEM1),
        .num_puzzle2 = Inv_RequestItem(O_PUZZLE_ITEM2),
        .num_puzzle3 = Inv_RequestItem(O_PUZZLE_ITEM3),
        .num_puzzle4 = Inv_RequestItem(O_PUZZLE_ITEM4),
        .num_key1 = Inv_RequestItem(O_KEY_ITEM1),
        .num_key2 = Inv_RequestItem(O_KEY_ITEM2),
        .num_key3 = Inv_RequestItem(O_KEY_ITEM3),
        .num_key4 = Inv_RequestItem(O_KEY_ITEM4),
        .num_leadbar = Inv_RequestItem(O_LEADBAR_ITEM),
        0
    };

    Savegame_Legacy_Write(&item_stats, sizeof(item_stats));

    Savegame_Legacy_Write(&g_FlipStatus, sizeof(int32_t));
    for (int i = 0; i < MAX_FLIP_MAPS; i++) {
        int8_t flag = g_FlipMapTable[i] >> 8;
        Savegame_Legacy_Write(&flag, sizeof(int8_t));
    }

    for (int i = 0; i < g_NumberCameras; i++) {
        Savegame_Legacy_Write(&g_Camera.fixed[i].flags, sizeof(int16_t));
    }

    for (int i = 0; i < g_LevelItemCount; i++) {
        ITEM_INFO *item = &g_Items[i];
        OBJECT_INFO *obj = &g_Objects[item->object_number];

        if (obj->save_position) {
            Savegame_Legacy_Write(&item->pos, sizeof(PHD_3DPOS));
            Savegame_Legacy_Write(&item->room_number, sizeof(int16_t));
            Savegame_Legacy_Write(&item->speed, sizeof(int16_t));
            Savegame_Legacy_Write(&item->fall_speed, sizeof(int16_t));
        }

        if (obj->save_anim) {
            Savegame_Legacy_Write(&item->current_anim_state, sizeof(int16_t));
            Savegame_Legacy_Write(&item->goal_anim_state, sizeof(int16_t));
            Savegame_Legacy_Write(&item->required_anim_state, sizeof(int16_t));
            Savegame_Legacy_Write(&item->anim_number, sizeof(int16_t));
            Savegame_Legacy_Write(&item->frame_number, sizeof(int16_t));
        }

        if (obj->save_hitpoints) {
            Savegame_Legacy_Write(&item->hit_points, sizeof(int16_t));
        }

        if (obj->save_flags) {
            uint16_t flags = item->flags + item->active + (item->status << 1)
                + (item->gravity_status << 3) + (item->collidable << 4);
            if (obj->intelligent && item->data) {
                flags |= SAVE_CREATURE;
            }
            Savegame_Legacy_Write(&flags, sizeof(uint16_t));
            Savegame_Legacy_Write(&item->timer, sizeof(int16_t));
            if (flags & SAVE_CREATURE) {
                CREATURE_INFO *creature = item->data;
                Savegame_Legacy_Write(
                    &creature->head_rotation, sizeof(int16_t));
                Savegame_Legacy_Write(
                    &creature->neck_rotation, sizeof(int16_t));
                Savegame_Legacy_Write(&creature->maximum_turn, sizeof(int16_t));
                Savegame_Legacy_Write(&creature->flags, sizeof(int16_t));
                Savegame_Legacy_Write(&creature->mood, sizeof(int32_t));
            }
        }
    }

    Savegame_Legacy_WriteLara(&g_Lara);

    Savegame_Legacy_Write(&g_FlipEffect, sizeof(int32_t));
    Savegame_Legacy_Write(&g_FlipTimer, sizeof(int32_t));

    File_Write(buffer, sizeof(char), m_SGBufPos, fp);
    Memory_FreePointer(&buffer);
}

bool Savegame_Legacy_UpdateDeathCounters(MYFILE *fp, GAME_INFO *game_info)
{
    return false;
}
