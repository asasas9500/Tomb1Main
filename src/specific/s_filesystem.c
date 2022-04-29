#include "specific/s_filesystem.h"

#include "log.h"

#include <SDL2/SDL.h>
#include <assert.h>
#include <string.h>

#if defined(_WIN32)
    #include <direct.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

const char *m_GameDir = NULL;

const char *S_File_GetGameDirectory(void)
{
    if (!m_GameDir) {
        m_GameDir = SDL_GetBasePath();
        if (!m_GameDir) {
            LOG_ERROR("Can't get module handle");
            return NULL;
        }
    }
    return m_GameDir;
}

void S_File_CreateDirectory(const char *path)
{
    assert(path);
#if defined(_WIN32)
    _mkdir(path);
#else
    mkdir(path, 0664);
#endif
}
