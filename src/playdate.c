#include "playdate.h"
#include "playdate_sys.h"

#include "DOOM.h"
#include "doomdef.h"

PlaydateAPI* playdate = NULL;
LCDBitmap* screen_bitmap;
uint8_t* bw_bitmap;

static int update(void* userdata);
// static void screenBufferToLCDBitmap(const unsigned char* buffer);

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
        bw_bitmap = playdate_malloc(sizeof(uint8_t) * SCREENWIDTH * SCREENHEIGHT);

        doom_init(1, NULL, 0);

        playdate->system->setUpdateCallback(update, playdate);
    }

    return 0;
}

static int update(void* userdata) {
    doom_update();
    // screenBufferToLCDBitmap(doom_get_framebuffer(4));
    // playdate->graphics->drawBitmap(screen_bitmap, 0, 0, 0);

    playdate->system->drawFPS(0, 0);
    return 1;
}

/*
static void screenBufferToLCDBitmap(const unsigned char* buffer)
{
    // iterate over Doom's colored bitmap and map each uint32_t pixel to either
    // black or white, based on a simple threshold. Map the results to a uint8_t
    // map of the same length (i.e. number of pixels)
    // TODO: Replace this with hopefully performant dithering techniques
    const uint32_t THRESHOLD32 = 0x44444444;

    for (int y = 0; y < SCREENHEIGHT; y++)
    {
        for (int x = 0; x < SCREENWIDTH; x++)
        {
            uint8_t r = buffer[y * SCREENWIDTH * 4 + x * 4 + 0];
            uint8_t g = buffer[y * SCREENWIDTH * 4 + x * 4 + 1];
            uint8_t b = buffer[y * SCREENWIDTH * 4 + x * 4 + 2];
            uint8_t a = buffer[y * SCREENWIDTH * 4 + x * 4 + 3];

            // black-white decision based on threshold
            uint32_t combined = ((uint32_t)r << 24) | ((uint32_t)g << 16) | ((uint32_t)b << 8) | a;
            uint8_t color = (combined < THRESHOLD32) ? 0 : 1;

            bw_bitmap[y * SCREENWIDTH + x] = color;
        }
    }

    // reset Playdate's screen bitmap and get screen_data pointer
    playdate->graphics->clearBitmap(screen_bitmap, 0);

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
}
*/