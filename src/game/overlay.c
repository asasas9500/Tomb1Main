#include "game/overlay.h"

#include "config.h"
#include "game/clock.h"
#include "game/output.h"
#include "game/screen.h"
#include "game/text.h"
#include "game/viewport.h"
#include "global/const.h"
#include "global/types.h"
#include "global/vars.h"

#include <stdio.h>

#define COLOR_STEPS 5
#define MAX_PICKUP_COLUMNS 4
#define MAX_PICKUPS 16
#define BLINK_THRESHOLD 20

static TEXTSTRING *m_AmmoText = NULL;
static TEXTSTRING *m_FPSText = NULL;
static int16_t m_BarOffsetY[6] = { 0 };
static DISPLAYPU m_Pickups[MAX_PICKUPS] = { 0 };
static int32_t m_OldGameTimer = 0;

static RGBA8888 m_ColorBarMap[][COLOR_STEPS] = {
    // gold
    { { 112, 92, 44, 255 },
      { 164, 120, 72, 255 },
      { 112, 92, 44, 255 },
      { 88, 68, 0, 255 },
      { 80, 48, 20, 255 } },
    // blue
    { { 100, 116, 100, 255 },
      { 92, 160, 156, 255 },
      { 100, 116, 100, 255 },
      { 76, 80, 76, 255 },
      { 48, 48, 48, 255 } },
    // grey
    { { 88, 100, 88, 255 },
      { 116, 132, 116, 255 },
      { 88, 100, 88, 255 },
      { 76, 80, 76, 255 },
      { 48, 48, 48, 255 } },
    // red
    { { 160, 40, 28, 255 },
      { 184, 44, 32, 255 },
      { 160, 40, 28, 255 },
      { 124, 32, 32, 255 },
      { 84, 20, 32, 255 } },
    // silver
    { { 150, 150, 150, 255 },
      { 230, 230, 230, 255 },
      { 200, 200, 200, 255 },
      { 140, 140, 140, 255 },
      { 100, 100, 100, 255 } },
    // green
    { { 100, 190, 20, 255 },
      { 130, 230, 30, 255 },
      { 100, 190, 20, 255 },
      { 90, 150, 15, 255 },
      { 80, 110, 10, 255 } },
    // gold2
    { { 220, 170, 0, 255 },
      { 255, 200, 0, 255 },
      { 220, 170, 0, 255 },
      { 185, 140, 0, 255 },
      { 150, 100, 0, 255 } },
    // blue2
    { { 0, 170, 220, 255 },
      { 0, 200, 255, 255 },
      { 0, 170, 220, 255 },
      { 0, 140, 185, 255 },
      { 0, 100, 150, 255 } },
    // pink
    { { 220, 140, 170, 255 },
      { 255, 150, 200, 255 },
      { 210, 130, 160, 255 },
      { 165, 100, 120, 255 },
      { 120, 60, 70, 255 } },
};

typedef struct BAR_INFO {
    BAR_TYPE type;
    int32_t value;
    int32_t max_value;
    bool show;
    BAR_SHOW_MODE show_mode;
    bool blink;
    int32_t blink_counter;
    int32_t timer;
    int32_t color;
    BAR_LOCATION location;
} BAR_INFO;

static BAR_INFO m_HealthBar = { 0 };
static BAR_INFO m_AirBar = { 0 };
static BAR_INFO m_EnemyBar = { 0 };

static void Overlay_BarSetupHealth(void);
static void Overlay_BarSetupAir(void);
static void Overlay_BarSetupEnemy(void);
static void Overlay_BarBlink(BAR_INFO *bar_info);
static int32_t Overlay_BarGetPercent(BAR_INFO *bar_info);
static void Overlay_BarGetLocation(
    BAR_LOCATION bar_location, int32_t width, int32_t height, int32_t *x,
    int32_t *y);
static void Overlay_BarDraw(BAR_INFO *bar_info);
static void Overlay_OnAmmoTextRemoval(const TEXTSTRING *textstring);
static void Overlay_OnFPSTextRemoval(const TEXTSTRING *textstring);

static void Overlay_BarSetupHealth(void)
{
    m_HealthBar.type = BT_LARA_HEALTH;
    m_HealthBar.value = 0;
    m_HealthBar.max_value = LARA_HITPOINTS;
    m_HealthBar.show_mode = g_Config.healthbar_showing_mode;
    m_HealthBar.show = false;
    m_HealthBar.blink = false;
    m_HealthBar.blink_counter = 0;
    m_HealthBar.timer = 40;
    m_HealthBar.color = g_Config.healthbar_color;
    m_HealthBar.location = g_Config.healthbar_location;
}

static void Overlay_BarSetupAir(void)
{
    m_AirBar.type = BT_LARA_AIR;
    m_AirBar.value = LARA_AIR;
    m_AirBar.max_value = LARA_AIR;
    m_AirBar.show_mode = g_Config.airbar_showing_mode;
    m_AirBar.show = false;
    m_AirBar.blink = false;
    m_AirBar.blink_counter = 0;
    m_AirBar.timer = 0;
    m_AirBar.color = g_Config.airbar_color;
    m_AirBar.location = g_Config.airbar_location;
}

static void Overlay_BarSetupEnemy(void)
{
    m_EnemyBar.type = BT_ENEMY_HEALTH;
    m_EnemyBar.value = 0;
    m_EnemyBar.max_value = 0;
    m_EnemyBar.show_mode = g_Config.healthbar_showing_mode;
    m_EnemyBar.show = g_Config.enable_enemy_healthbar;
    m_EnemyBar.blink = false;
    m_EnemyBar.blink_counter = 0;
    m_EnemyBar.timer = 0;
    m_EnemyBar.color = g_Config.enemy_healthbar_color;
    m_EnemyBar.location = g_Config.enemy_healthbar_location;
}

static int32_t Overlay_BarGetPercent(BAR_INFO *bar_info)
{
    return bar_info->value * 100 / bar_info->max_value;
}

static void Overlay_BarBlink(BAR_INFO *bar_info)
{
    if (bar_info->show_mode == BSM_PS1 || bar_info->type == BT_ENEMY_HEALTH) {
        bar_info->blink = false;
        return;
    }

    int32_t percent = Overlay_BarGetPercent(bar_info);
    const int32_t blink_interval = 20;
    int32_t blink_time = bar_info->blink_counter++ % blink_interval;

    bar_info->blink =
        percent <= BLINK_THRESHOLD && blink_time > blink_interval / 2;
}

static void Overlay_BarGetLocation(
    BAR_LOCATION bar_location, int32_t width, int32_t height, int32_t *x,
    int32_t *y)
{
    const int32_t screen_margin_h = 20;
    const int32_t screen_margin_v = 18;
    const int32_t bar_spacing = 8;

    if (bar_location == BL_TOP_LEFT || bar_location == BL_BOTTOM_LEFT) {
        *x = screen_margin_h;
    } else if (
        bar_location == BL_TOP_RIGHT || bar_location == BL_BOTTOM_RIGHT) {
        *x = Screen_GetResWidthDownscaled() - width * g_Config.ui.bar_scale
            - screen_margin_h;
    } else {
        *x = (Screen_GetResWidthDownscaled() - width) / 2;
    }

    if (bar_location == BL_TOP_LEFT || bar_location == BL_TOP_CENTER
        || bar_location == BL_TOP_RIGHT) {
        *y = screen_margin_v + m_BarOffsetY[bar_location];
    } else {
        *y = Screen_GetResHeightDownscaled() - height * g_Config.ui.bar_scale
            - screen_margin_v - m_BarOffsetY[bar_location];
    }

    m_BarOffsetY[bar_location] += height + bar_spacing;
}

static void Overlay_BarDraw(BAR_INFO *bar_info)
{
    const RGBA8888 rgb_bgnd = { 0, 0, 0, 255 };
    const RGBA8888 rgb_border_light = { 128, 128, 128, 255 };
    const RGBA8888 rgb_border_dark = { 64, 64, 64, 255 };

    int32_t width = 100;
    int32_t height = 5;

    int32_t x = 0;
    int32_t y = 0;
    Overlay_BarGetLocation(bar_info->location, width, height, &x, &y);

    int32_t padding = Screen_GetResWidth() <= 800 ? 1 : 2;
    int32_t border = 1;
    int32_t sx = Screen_GetRenderScale(x) - padding;
    int32_t sy = Screen_GetRenderScale(y) - padding;
    int32_t sw =
        Screen_GetRenderScale(width) * g_Config.ui.bar_scale + padding * 2;
    int32_t sh =
        Screen_GetRenderScale(height) * g_Config.ui.bar_scale + padding * 2;

    // border
    Output_DrawScreenFlatQuad(
        sx - border, sy - border, sw + border, sh + border, rgb_border_dark);
    Output_DrawScreenFlatQuad(
        sx, sy, sw + border, sh + border, rgb_border_light);

    // background
    Output_DrawScreenFlatQuad(sx, sy, sw, sh, rgb_bgnd);

    int32_t percent = Overlay_BarGetPercent(bar_info);

    // Check if bar should flash or not
    Overlay_BarBlink(bar_info);

    if (percent && !bar_info->blink) {
        width = width * percent / 100;

        sx = Screen_GetRenderScale(x);
        sy = Screen_GetRenderScale(y);
        sw = Screen_GetRenderScale(width) * g_Config.ui.bar_scale;
        sh = Screen_GetRenderScale(height) * g_Config.ui.bar_scale;

        if (g_Config.enable_smooth_bars) {
            for (int i = 0; i < COLOR_STEPS - 1; i++) {
                RGBA8888 c1 = m_ColorBarMap[bar_info->color][i];
                RGBA8888 c2 = m_ColorBarMap[bar_info->color][i + 1];
                int32_t lsy = sy + i * sh / (COLOR_STEPS - 1);
                int32_t lsh = sy + (i + 1) * sh / (COLOR_STEPS - 1) - lsy;
                Output_DrawScreenGradientQuad(sx, lsy, sw, lsh, c1, c1, c2, c2);
            }
        } else {
            for (int i = 0; i < COLOR_STEPS; i++) {
                RGBA8888 color = m_ColorBarMap[bar_info->color][i];
                int32_t lsy = sy + i * sh / COLOR_STEPS;
                int32_t lsh = sy + (i + 1) * sh / COLOR_STEPS - lsy;
                Output_DrawScreenFlatQuad(sx, lsy, sw, lsh, color);
            }
        }
    }
}

static void Overlay_OnAmmoTextRemoval(const TEXTSTRING *textstring)
{
    m_AmmoText = NULL;
}

static void Overlay_OnFPSTextRemoval(const TEXTSTRING *textstring)
{
    m_FPSText = NULL;
}

void Overlay_Init(void)
{
    for (int i = 0; i < MAX_PICKUPS; i++) {
        m_Pickups[i].duration = 0;
    }

    Overlay_BarSetupHealth();
    Overlay_BarSetupAir();
    Overlay_BarSetupEnemy();
}

void Overlay_BarSetHealthTimer(int16_t timer)
{
    m_HealthBar.timer = timer;
}

void Overlay_BarHealthTimerTick(void)
{
    m_HealthBar.timer--;
    CLAMPL(m_HealthBar.timer, 0);
}

void Overlay_BarDrawHealth(void)
{
    static int32_t old_hit_points = 0;

    for (int i = 0; i < 6; i++) {
        m_BarOffsetY[i] = 0;
    }

    m_HealthBar.value = g_LaraItem->hit_points;
    CLAMP(m_HealthBar.value, 0, m_HealthBar.max_value);

    if (old_hit_points != m_HealthBar.value) {
        old_hit_points = m_HealthBar.value;
        m_HealthBar.timer = 40;
    }

    m_HealthBar.show = m_HealthBar.timer > 0 || m_HealthBar.value <= 0
        || g_Lara.gun_status == LGS_READY;
    switch (m_HealthBar.show_mode) {
    case BSM_FLASHING_OR_DEFAULT:
        m_HealthBar.show |= m_HealthBar.value
            <= (m_HealthBar.max_value * BLINK_THRESHOLD) / 100;
        break;
    case BSM_FLASHING_ONLY:
        m_HealthBar.show = m_HealthBar.value
            <= (m_HealthBar.max_value * BLINK_THRESHOLD) / 100;
        break;
    case BSM_ALWAYS:
        m_HealthBar.show = true;
        break;
    case BSM_NEVER:
        m_HealthBar.show = false;
        break;
    default:
        break;
    }
    if (!m_HealthBar.show) {
        return;
    }

    Overlay_BarDraw(&m_HealthBar);
}

void Overlay_BarDrawAir(void)
{
    m_AirBar.value = g_Lara.air;
    CLAMP(m_AirBar.value, 0, m_AirBar.max_value);

    m_AirBar.show = g_Lara.water_status == LWS_UNDERWATER
        || g_Lara.water_status == LWS_SURFACE;

    if (!m_AirBar.show) {
        return;
    }

    switch (m_AirBar.show_mode) {
    case BSM_DEFAULT:
        m_AirBar.show |=
            g_Lara.air <= (m_AirBar.max_value * BLINK_THRESHOLD) / 100;
        break;
    case BSM_FLASHING_ONLY:
        m_AirBar.show =
            g_Lara.air <= (m_AirBar.max_value * BLINK_THRESHOLD) / 100;
        break;
    case BSM_NEVER:
        m_AirBar.show = false;
        break;
    default:
        break;
    }
    if (!m_AirBar.show) {
        return;
    }

    Overlay_BarDraw(&m_AirBar);
}

void Overlay_BarDrawEnemy(void)
{
    if (!m_EnemyBar.show || !g_Lara.target) {
        return;
    }

    m_EnemyBar.value = g_Lara.target->hit_points;
    m_EnemyBar.max_value = g_Objects[g_Lara.target->object_number].hit_points
        * ((g_GameInfo.bonus_flag & GBF_NGPLUS) ? 2 : 1);
    CLAMP(m_EnemyBar.value, 0, m_EnemyBar.max_value);

    Overlay_BarDraw(&m_EnemyBar);
}

void Overlay_DrawAmmoInfo(void)
{
    const double scale = 0.8;
    const int32_t text_height = 17 * scale;
    const int32_t text_offset_x = 3;
    const int32_t screen_margin_h = 20;
    const int32_t screen_margin_v = 18;

    char ammostring[80] = "";

    if (g_Lara.gun_status != LGS_READY || g_OverlayFlag <= 0
        || (g_GameInfo.bonus_flag & GBF_NGPLUS)) {
        if (m_AmmoText) {
            Text_Remove(m_AmmoText);
            m_AmmoText = NULL;
        }
        return;
    }

    switch (g_Lara.gun_type) {
    case LGT_PISTOLS:
        return;
    case LGT_MAGNUMS:
        sprintf(ammostring, "%5d B", g_Lara.magnums.ammo);
        break;
    case LGT_UZIS:
        sprintf(ammostring, "%5d C", g_Lara.uzis.ammo);
        break;
    case LGT_SHOTGUN:
        sprintf(ammostring, "%5d A", g_Lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
        break;
    default:
        return;
    }

    Overlay_MakeAmmoString(ammostring);

    if (m_AmmoText) {
        Text_ChangeText(m_AmmoText, ammostring);
    } else {
        m_AmmoText = Text_Create(
            -screen_margin_h - text_offset_x, text_height + screen_margin_v,
            ammostring);
        m_AmmoText->on_remove = Overlay_OnAmmoTextRemoval;
        Text_SetScale(m_AmmoText, PHD_ONE * scale, PHD_ONE * scale);
        Text_AlignRight(m_AmmoText, 1);
    }

    m_AmmoText->pos.y = m_BarOffsetY[BL_TOP_RIGHT]
        ? text_height + screen_margin_v + m_BarOffsetY[BL_TOP_RIGHT]
        : text_height + screen_margin_v;
}

void Overlay_DrawPickups(void)
{
    int16_t time =
        g_GameInfo.current[g_CurrentLevel].stats.timer - m_OldGameTimer;
    m_OldGameTimer = g_GameInfo.current[g_CurrentLevel].stats.timer;

    if (time > 0 && time < 60) {
        int32_t sprite_height =
            MIN(ViewPort_GetWidth(), ViewPort_GetHeight() * 320 / 200) / 10;
        int32_t sprite_width = sprite_height * 4 / 3;
        int32_t y = ViewPort_GetHeight() - sprite_height;
        int32_t x = ViewPort_GetWidth() - sprite_height;
        for (int i = 0; i < MAX_PICKUPS; i++) {
            DISPLAYPU *pu = &m_Pickups[i];
            pu->duration -= time;
            if (pu->duration <= 0) {
                pu->duration = 0;
            } else {
                Output_DrawUISprite(
                    x, y, Screen_GetRenderScaleGLRage(12288), pu->sprnum, 4096);

                if (i % MAX_PICKUP_COLUMNS == MAX_PICKUP_COLUMNS - 1) {
                    x = ViewPort_GetWidth() - sprite_height;
                    y -= sprite_height;
                } else {
                    x -= sprite_width;
                }
            }
        }
    }
}

void Overlay_DrawFPSInfo(void)
{
    static int32_t elapsed = 0;

    if (g_Config.rendering.enable_fps_counter) {
        if (Clock_GetMS() - elapsed >= 1000) {
            if (m_FPSText) {
                char fps_buf[20];
                sprintf(fps_buf, "%d FPS", g_FPSCounter);
                Text_ChangeText(m_FPSText, fps_buf);
            } else {
                char fps_buf[20];
                sprintf(fps_buf, "? FPS");
                m_FPSText = Text_Create(10, 30, fps_buf);
                m_FPSText->on_remove = Overlay_OnFPSTextRemoval;
            }
            g_FPSCounter = 0;
            elapsed = Clock_GetMS();
        }
    } else if (m_FPSText) {
        Text_Remove(m_FPSText);
        m_FPSText = NULL;
        g_FPSCounter = 0;
    }
}

void Overlay_DrawGameInfo(void)
{
    if (g_OverlayFlag > 0) {
        Overlay_BarDrawHealth();
        Overlay_BarDrawAir();
        Overlay_BarDrawEnemy();
        Overlay_DrawPickups();
    }

    Overlay_DrawAmmoInfo();
    Overlay_DrawFPSInfo();

    Text_Draw();
}

void Overlay_AddPickup(int16_t object_num)
{
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (m_Pickups[i].duration <= 0) {
            m_Pickups[i].duration = 75;
            m_Pickups[i].sprnum = g_Objects[object_num].mesh_index;
            return;
        }
    }
}

void Overlay_MakeAmmoString(char *string)
{
    char *c;

    for (c = string; *c != 0; c++) {
        if (*c == 32) {
            continue;
        } else if (*c - 'A' >= 0) {
            *c += 12 - 'A';
        } else {
            *c += 1 - '0';
        }
    }
}
