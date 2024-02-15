//
// Created by Fabian Hartmann.
// https://github.com/yesnocancel
//

#include "playdate_sfx.h"

#include "DOOM.h"

#define SAMPLES_PER_CHANNEL 512
#define PLAYDATE_AUDIOBUFFER_SIZE 4096

static int16_t playdateAudioBuffer[PLAYDATE_AUDIOBUFFER_SIZE] = {0};
static int audioBufferIndex = 0;

static int doomAudioCallback(void* context, int16_t* left, int16_t* right, int len);

void initPlaydateSoundSource(void) {
    playdate->sound->addSource(doomAudioCallback, NULL, 1); // 1 for stereo
}

static void refillAudioBuffer(int16_t* doomBuffer) {
    int idx = 0;
    for (int i = 0; i < 2 * SAMPLES_PER_CHANNEL; i += 2) {
        for (int j = 0; j < 4; j++) {
            playdateAudioBuffer[idx++] = doomBuffer[i];
            playdateAudioBuffer[idx++] = doomBuffer[i+1];
        }
    }
}

static int doomAudioCallback(void* context, int16_t* left, int16_t* right, int len) {
    if (audioBufferIndex == 0) {
        int16_t* soundBuffer = doom_get_sound_buffer();
        refillAudioBuffer(soundBuffer);
    }

    for (int i = 0; i < len; i++) {
        if (audioBufferIndex < PLAYDATE_AUDIOBUFFER_SIZE) {
            left[i] = playdateAudioBuffer[audioBufferIndex++];
            right[i] = playdateAudioBuffer[audioBufferIndex++];
        } else {
            left[i] = 0;
            right[i] = 0;
        }
    }

    if (audioBufferIndex >= PLAYDATE_AUDIOBUFFER_SIZE) {
        audioBufferIndex = 0;
    }

    return 1; // Meaningful data in left and right
}