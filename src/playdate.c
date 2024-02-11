#include "playdate.h"
#include "playdate_sys.h"
#include <stdio.h>

#include "DOOM.h"
#include "doomdef.h"
#include "doom_config.h"

PlaydateAPI* playdate = NULL;
LCDBitmap* lcdBitmap;

unsigned char framebuffer_grey[SCREENWIDTH * SCREENHEIGHT];

static int update(void* userdata);
static void handleInputs(void);
static void generateFrameBufferGrey(unsigned char* target, const unsigned char* framebuffer);
static void floydSteinbergDithering(unsigned char *image, int width, int height);
static void bayerDithering4x4(unsigned char *image, int width, int height);
static void atkinsonDithering(unsigned char *image, int width, int height);
static void voidAndClusterDithering(unsigned char *image, int width, int height);


static void screenBufferToLCDBitmap(LCDBitmap* lcd, const unsigned char* framebuffer);

static uint8_t DOOM_PLAYPAL_Grey[256] = {
          0, 24, 16, 75,255, 26, 19, 11,  7, 49, 37, 25, 17, 63, 55, 47,
        204,193,186,176,169,161,151,143,135,128,120,116,108,102, 94, 90,
         82, 78, 72, 66, 60, 56, 51, 47, 42, 38, 33, 32, 29, 23, 21, 20,
        239,233,227,221,218,212,206,202,196,188,180,172,164,156,148,144,
        137,129,123,118,111,104, 97, 92, 86, 78, 71, 65, 57, 49, 42, 35,
        239,231,223,219,211,203,199,191,183,179,171,167,159,151,147,139,
        131,127,119,111,107, 99, 91, 87, 79, 71, 67, 59, 55, 47, 39, 35,
        197,185,172,159,148,135,122,112,101, 88, 76, 63, 51, 38, 27, 17,
        171,163,155,147,139,133,127,119,111,103, 99, 91, 86, 78, 71, 67,
        135,122,110, 98, 85, 73, 62, 53,122,110,102, 93, 82, 73, 65, 57,
        239,208,181,154,128,101, 80, 59,255,229,207,184,162,142,120, 97,
         76, 71, 67, 64, 60, 57, 53, 49, 46, 41, 37, 34, 30, 27, 23, 20,
        233,205,180,155,130,102, 77, 52, 29, 25, 23, 20, 17, 14, 12,  9,
        255,239,223,210,195,182,166,153,142,137,128,121,112,104, 96, 91,
        255,250,246,242,238,234,229,225, 86, 79, 71, 60, 62, 50, 39, 31,
          9,  8,  6,  5,  3,  2,  1,  0,177,220,177,105, 85, 65, 45,124
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
        playdate->graphics->clear(kColorBlack);
        lcdBitmap = playdate->graphics->newBitmap(SCREENWIDTH, SCREENHEIGHT, kColorBlack);

        doom_init(1, NULL, 0);

        playdate->system->setUpdateCallback(update, playdate);
    }

    return 0;
}

static int update(void* userdata) {
    doom_update();
    handleInputs();

    generateFrameBufferGrey(framebuffer_grey, doom_get_framebuffer(1));
    // atkinsonDithering(framebuffer_grey, SCREENWIDTH, SCREENHEIGHT);
    // bayerDithering4x4(framebuffer_grey, SCREENWIDTH, SCREENHEIGHT);
    // floydSteinbergDithering(framebuffer_grey, SCREENWIDTH, SCREENHEIGHT);
    // voidAndClusterDithering(framebuffer_grey, SCREENWIDTH, SCREENHEIGHT);

    screenBufferToLCDBitmap(lcdBitmap, framebuffer_grey);
    // playdate->graphics->drawScaledBitmap(lcdBitmap, 8, 0, 1.2, 1.2);
    playdate->graphics->drawBitmap(lcdBitmap, 40, 20, 0);

    playdate->system->drawFPS(0, 0);
    return 1;
}

static void generateFrameBufferGrey(unsigned char* target, const unsigned char* source) {
    int poormansgamma = 100;

    for (int i = 0; i < SCREENWIDTH * SCREENHEIGHT; i++) {
        target[i] = DOOM_PLAYPAL_Grey[source[i]] < (255 - poormansgamma) ? DOOM_PLAYPAL_Grey[source[i]] + poormansgamma : 255;
    }
}

static void floydSteinbergDithering(unsigned char *image, int width, int height) {
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int oldPixel = image[y * width + x];
            int newPixel = oldPixel > 127 ? 255 : 0;
            image[y * width + x] = newPixel;

            int error = oldPixel - newPixel;

            if (x + 1 < width)
                image[y * width + x + 1] += error * 7 / 16;
            if (x - 1 >= 0 && y + 1 < height)
                image[(y + 1) * width + x - 1] += error * 3 / 16;
            if (y + 1 < height)
                image[(y + 1) * width + x] += error * 5 / 16;
            if (x + 1 < width && y + 1 < height)
                image[(y + 1) * width + x + 1] += error * 1 / 16;
        }
    }
}

static void bayerDithering4x4(unsigned char *image, int width, int height) {
    int bayerMatrix[4][4] = {
            {  1,  9,  3, 11 },
            { 13,  5, 15,  7 },
            {  4, 12,  2, 10 },
            { 16,  8, 14,  6 }
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y % 4;
            int j = x % 4;
            int mapValue = bayerMatrix[i][j];
            int threshold = 255 * mapValue / 17; // Scale factor for 4x4 Bayer matrix

            image[y * width + x] = (image[y * width + x] > threshold) ? 255 : 0;
        }
    }
}

void atkinsonDithering(unsigned char *image, int width, int height) {
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int oldPixel = image[y * width + x];
            int newPixel = oldPixel > 64 ? 255 : 0;
            image[y * width + x] = newPixel;

            int error = oldPixel - newPixel;

            // Distribute the error to neighboring pixels
            if (x + 1 < width)
                image[y * width + x + 1] += error * 1 / 8;
            if (x + 2 < width)
                image[y * width + x + 2] += error * 1 / 8;
            if (x - 1 >= 0 && y + 1 < height)
                image[(y + 1) * width + x - 1] += error * 1 / 8;
            if (x < width && y + 1 < height)
                image[(y + 1) * width + x] += error * 1 / 8;
            if (x + 1 < width && y + 1 < height)
                image[(y + 1) * width + x + 1] += error * 1 / 8;
            if (y + 2 < height)
                image[(y + 2) * width + x] += error * 1 / 8;
        }
    }
}

void voidAndClusterDithering(unsigned char *image, int width, int height) {
    int thresholdMap[4][4] = {
            { 15, 135, 45, 165 },
            { 195, 75, 225, 105 },
            { 60, 180, 30, 150 },
            { 240, 120, 210, 90 }
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int i = y % 4;
            int j = x % 4;
            int threshold = thresholdMap[i][j];

            int oldPixel = image[y * width + x];
            int newPixel = oldPixel > threshold ? 255 : 0;
            image[y * width + x] = newPixel;

            int error = oldPixel - newPixel;

            // Simple error diffusion
            if (x + 1 < width)
                image[y * width + x + 1] += error / 4;
            if (y + 1 < height)
                image[(y + 1) * width + x] += error / 4;
            if (x + 1 < width && y + 1 < height)
                image[(y + 1) * width + x + 1] += error / 4;
            if (x > 0 && y + 1 < height)
                image[(y + 1) * width + x - 1] += error / 4;
        }
    }
}


static void screenBufferToLCDBitmap(LCDBitmap* lcd, const uint8_t* framebuffer)
{
    uint8_t* screen_data = 0;
    playdate->graphics->getBitmapData(lcd, 0, 0, 0, 0, &screen_data);

    // populate Playdate's screen bitmap with values from the black-white
    // bitmap we created above. Where 8 pixels were 8 bytes before, they
    // now fit into a single byte of the screen bitmap, so we need to do some bit operations
    for (int z = 0; z < SCREENWIDTH * SCREENHEIGHT / 8; z++) {
        uint8_t result = 0;
        for (int i = 0; i < 8; ++i) {
            // unsigned char pd_pixel = (framebuffer[z * 8 + i]);
            // result = (result << 1) | (pd_pixel & 0x01);

            result = (result << 1) | ((framebuffer[z * 8 + i]) & 1);
        }
        screen_data[z] = result;
    }
}

static void handleInputs() {
    PDButtons current;
    PDButtons pushed;
    PDButtons released;

    playdate->system->getButtonState(&current, &pushed, &released);

    if (current + pushed + released == 0) {
        doom_key_up(DOOM_KEY_ENTER);
        doom_key_up(DOOM_KEY_SPACE);
        doom_key_up(DOOM_KEY_CTRL);
        doom_key_up(DOOM_KEY_UP_ARROW);
        doom_key_up(DOOM_KEY_DOWN_ARROW);
        doom_key_up(DOOM_KEY_LEFT_ARROW);
        doom_key_up(DOOM_KEY_RIGHT_ARROW);
    }

    if ((current & kButtonA) | (pushed & kButtonA)) {
        doom_key_down(DOOM_KEY_ENTER);
        doom_key_down(DOOM_KEY_SPACE);
    }
    if ((current & kButtonB) | (pushed & kButtonB)) {
        doom_key_down(DOOM_KEY_CTRL);
    }
    if ((current & kButtonUp) | (pushed & kButtonUp)) {
        doom_key_down(DOOM_KEY_UP_ARROW);
    }
    if ((current & kButtonDown) | (pushed & kButtonDown)) {
        doom_key_down(DOOM_KEY_DOWN_ARROW);
    }
    if ((current & kButtonLeft) | (pushed & kButtonLeft)) {
        doom_key_down(DOOM_KEY_LEFT_ARROW);
    }
    if ((current & kButtonRight) | (pushed & kButtonRight)) {
        doom_key_down(DOOM_KEY_RIGHT_ARROW);
    }

    if ((released & kButtonA)) {
        doom_key_up(DOOM_KEY_ENTER);
        doom_key_up(DOOM_KEY_SPACE);
    }
    if ((released & kButtonB)) {
        doom_key_up(DOOM_KEY_CTRL);
    }
    if ((released & kButtonUp)) {
        doom_key_up(DOOM_KEY_UP_ARROW);
    }
    if ((released & kButtonDown)) {
        doom_key_up(DOOM_KEY_DOWN_ARROW);
    }
    if ((released & kButtonLeft)) {
        doom_key_up(DOOM_KEY_LEFT_ARROW);
    }
    if ((released & kButtonRight)) {
        doom_key_up(DOOM_KEY_RIGHT_ARROW);
    }


}
