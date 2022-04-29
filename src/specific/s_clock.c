#include "specific/s_clock.h"

#include "config.h"
#include "global/vars.h"

#include <windows.h>

static LONGLONG m_Ticks = 0;
static double m_Frequency = 0.0;

static void m_UpdateTicks(void);

static void m_UpdateTicks(void)
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    m_Ticks = counter.QuadPart;
}

bool S_Clock_Init(void)
{
    LARGE_INTEGER ticks_per_second;
    if (!QueryPerformanceFrequency(&ticks_per_second)) {
        return false;
    }

    m_Frequency = (double)ticks_per_second.QuadPart / (double)TICKS_PER_SECOND;
    m_UpdateTicks();
    return true;
}

int32_t S_Clock_GetMS(void)
{
    return GetTickCount();
}

int32_t S_Clock_Sync(void)
{
    LONGLONG last_ticks = m_Ticks;
    m_UpdateTicks();
    return ((double)(m_Ticks - last_ticks) / m_Frequency);
}

int32_t S_Clock_SyncTicks(int32_t target)
{
    double elapsed = 0.0;
    LONGLONG last_ticks = m_Ticks;
    do {
        m_UpdateTicks();
        elapsed = (double)(m_Ticks - last_ticks) / m_Frequency;
    } while (elapsed < (double)target);
    return elapsed;
}
