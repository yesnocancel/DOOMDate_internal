#include "playdate.h"
#include "playdate_sys.h"
#include <stdio.h>

#include "DOOM.h"
#include "doomdef.h"
#include "doom_config.h"

PlaydateAPI* playdate = NULL;
LCDBitmap* screen_bitmap;

static int update(void* userdata);
static void handleInputs(void);
static void screenBufferToLCDBitmap(const unsigned char* buffer);

static uint8_t playdatePaletteOriginal[256] = {
        0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0
};

static uint8_t playdatePaletteOptimized[256] = {
        0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
        1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,
        1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1
};

int eventHandler(PlaydateAPI* pd, PDSystemEvent event, uint32_t arg)
{
    (void)arg;

    if ( event == kEventInit )
    {
        playdate = pd;
        register_playdate_sys_functions();
        playdate->system->resetElapsedTime();
        playdate->display->setRefreshRate(35);
        screen_bitmap = playdate->graphics->newBitmap(SCREENWIDTH, SCREENHEIGHT, kColorWhite);

        // bw_bitmap = playdate_malloc(sizeof(uint8_t) * SCREENWIDTH * SCREENHEIGHT);

        doom_init(1, NULL, 0);

        playdate->system->setUpdateCallback(update, playdate);
    }

    return 0;
}

static int update(void* userdata) {
    handleInputs();
    doom_update();
    screenBufferToLCDBitmap(doom_get_framebuffer(1));
    playdate->graphics->drawBitmap(screen_bitmap, 0, 0, 0);


    playdate->system->drawFPS(0, 0);
    return 1;
}

static void screenBufferToLCDBitmap(const uint8_t* buffer)
{
    uint8_t* screen_data = 0;
    playdate->graphics->getBitmapData(screen_bitmap, 0, 0, 0, 0, &screen_data);

    for (int z = 0; z < SCREENWIDTH * SCREENHEIGHT / 8; z++) {
        uint8_t result = 0;
        for (int i = 0; i < 8; ++i) {
            uint8_t color = playdatePaletteOptimized[buffer[z * 8 + i]];
            result = (result << 1) | (color & 0x01);
        }
        screen_data[z] = result;
    }

    // This is the less optimized version from above:

    /*
    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
        bw_bitmap[i] = playdatePaletteOptimized[buffer[i]];
    }

    uint8_t* screen_data = NULL;
    playdate->graphics->getBitmapData(screen_bitmap, 0, 0, 0, 0, &screen_data);


    // populate Playdate's screen bitmap with values from the black-white
    // bitmap we created above. Where 8 pixels were 8 bytes before, they
    // now fit into a single byte of the screen bitmap, so we need to do some bit operations
    int datalength = SCREENWIDTH * SCREENHEIGHT / 8;

    for (int z = 0; z < datalength; z++) {
        uint8_t result = 0;
        for (int i = 0; i < 8; ++i) {
            uint8_t color = bw_bitmap[z * 8 + i];
            result = (result << 1) | (color & 0x01);
        }
        screen_data[z] = result;
    }
    */
}

static void handleInputs() {
    PDButtons current;
    PDButtons pushed;
    PDButtons released;

    playdate->system->getButtonState(&current, &pushed, &released);

    switch (released & 0xFF) {
        case kButtonA:
            doom_key_up(DOOM_KEY_ENTER);
            doom_key_up(DOOM_KEY_SPACE);
            break;
        case kButtonB:
            doom_key_up(DOOM_KEY_CTRL);
            break;
        case kButtonUp:
            doom_key_up(DOOM_KEY_UP_ARROW);
            break;
        case kButtonDown:
            doom_key_up(DOOM_KEY_DOWN_ARROW);
            break;
        case kButtonLeft:
            doom_key_up(DOOM_KEY_LEFT_ARROW);
            break;
        case kButtonRight:
            doom_key_up(DOOM_KEY_RIGHT_ARROW);
            break;
    }

    switch (pushed & 0xFF) {
        case kButtonA:
            doom_key_down(DOOM_KEY_ENTER);
            doom_key_down(DOOM_KEY_SPACE);
            break;
        case kButtonB:
            doom_key_down(DOOM_KEY_CTRL);
            break;
        case kButtonUp:
            doom_key_down(DOOM_KEY_UP_ARROW);
            break;
        case kButtonDown:
            doom_key_down(DOOM_KEY_DOWN_ARROW);
            break;
        case kButtonLeft:
            doom_key_down(DOOM_KEY_LEFT_ARROW);
            break;
        case kButtonRight:
            doom_key_down(DOOM_KEY_RIGHT_ARROW);
            break;
    }
}
