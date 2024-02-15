//
// Created by Fabian Hartmann.
// https://github.com/yesnocancel
//

#include "playdate_input.h"

#include "DOOM.h"

void handleInputs(PlaydateAPI* playdate) {
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