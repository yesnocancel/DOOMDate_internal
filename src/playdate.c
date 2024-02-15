#include <pd_api.h>

#include "playdate_gfx.h"
#include "playdate_input.h"
#include "playdate_sfx.h"
#include "playdate_sys.h"
#include "DOOM.h"

PlaydateAPI* playdate;

#define SHOW_FPS 0

static int update(void* userdata);

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
    (void)arg;

    if ( event == kEventInit )
    {
        playdate = pd;

        registerPlaydateSysFunctions();
        initPlaydateGraphics();
        initPlaydateSoundSource();

        playdate->system->resetElapsedTime();
        playdate->system->setUpdateCallback(update, playdate);

        doom_init(1, NULL, 0);
    }

    return 0;
}

static int update(void* userdata)
{
    handleInputs();
    doom_update();
    refreshScreen();
#if SHOW_FPS
    playdate->system->drawFPS(0, 0);
#endif
    return 1;
}
