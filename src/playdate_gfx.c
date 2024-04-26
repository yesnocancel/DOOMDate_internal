//
// Created by Fabian Hartmann.
// https://github.com/yesnocancel
//

#include "playdate_gfx.h"

#include "DOOM.h"

#define GAMMA 0.8f

#define DOOM_PALETTE_SIZE 256
#define DOOM_SCREENWIDTH 320
#define DOOM_SCREENHEIGHT 200

#define PLAYDATE_SCREENWIDTH 400
#define PLAYDATE_SCREENHEIGHT 240
#define PLAYDATE_REFRESH_RATE 35

extern PlaydateAPI* playdate;
LCDBitmap* lcdBitmap;
unsigned char framebuffer_grey[DOOM_SCREENWIDTH * DOOM_SCREENHEIGHT];

uint8_t DOOM_PLAYPAL_Grey[DOOM_PALETTE_SIZE] = {
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

static void calculateGammaPalette(float gamma);
static void generateFrameBufferGrey(unsigned char* target, const unsigned char* framebuffer);
static void screenBufferToLCDBitmap(LCDBitmap* lcd, const unsigned char* framebuffer);
static void bayerDithering4x4(unsigned char *image, int width, int height);

void initPlaydateGraphics(void) {
    calculateGammaPalette(GAMMA);

    lcdBitmap = playdate->graphics->newBitmap(DOOM_SCREENWIDTH, DOOM_SCREENHEIGHT, kColorBlack);

    playdate->system->resetElapsedTime();
    playdate->display->setRefreshRate(PLAYDATE_REFRESH_RATE);
    playdate->graphics->clear(kColorBlack);
}

void refreshScreen(void) {
    generateFrameBufferGrey(framebuffer_grey, doom_get_framebuffer(1));
    bayerDithering4x4(framebuffer_grey, DOOM_SCREENWIDTH, DOOM_SCREENHEIGHT);
    screenBufferToLCDBitmap(lcdBitmap, framebuffer_grey);
    // playdate->graphics->drawScaledBitmap(lcdBitmap, 8, 0, 1.2, 1.2);
    playdate->graphics->drawBitmap(lcdBitmap, 40, 20, 0);
}

static void calculateGammaPalette(float gamma) {
    for (int i = 0; i < 256; i++) {
        float value = 255.0f * powf((float)DOOM_PLAYPAL_Grey[i] / 255.0f, gamma);
        DOOM_PLAYPAL_Grey[i] = (uint8_t)(value + 0.5f); // Adding 0.5 for rounding to nearest integer
    }
}

static void generateFrameBufferGrey(unsigned char* target, const unsigned char* source) {
    for (int i = 0; i < DOOM_SCREENWIDTH * DOOM_SCREENHEIGHT; i++) {
        target[i] = DOOM_PLAYPAL_Grey[source[i]];
    }
}

static void bayerDithering4x4(unsigned char *image, int width, int height) {
    int bayerMatrix[4][4] = {
            {   0, 128,  32, 160 },
            { 192,  64, 224,  96 },
            {  48, 176,  16, 144 },
            { 240, 112, 208,  80 }
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int threshold = bayerMatrix[y % 4][x % 4];

            image[y * width + x] = (image[y * width + x] > threshold) ? 255 : 0;
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
    for (int z = 0; z < DOOM_SCREENWIDTH * DOOM_SCREENHEIGHT / 8; z++) {
        uint8_t result = 0;
        for (int i = 0; i < 8; ++i) {
            result = (result << 1) | ((framebuffer[z * 8 + i]) & 1);
        }
        screen_data[z] = result;
    }
}
