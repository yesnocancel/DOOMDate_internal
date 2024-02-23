//
// Created by Fabian Hartmann.
// https://github.com/yesnocancel
//

#include "playdate_input.h"
#include "DOOM.h"

extern player_t players[MAXPLAYERS];

static void cycleWeapons(weapontype_t currentWeapon);

void playdateHandleInputs(void) {
    PDButtons current;
    PDButtons pushed;
    PDButtons released;
    playdate->system->getButtonState(&current, &pushed, &released);
    float crankChange = playdate->system->getCrankChange();

    if (current + pushed + released + (int)crankChange == 0) {
        doom_key_up(DOOM_KEY_ENTER);
        doom_key_up(DOOM_KEY_SPACE);
        doom_key_up(DOOM_KEY_CTRL);
        doom_key_up(DOOM_KEY_UP_ARROW);
        doom_key_up(DOOM_KEY_DOWN_ARROW);
        doom_key_up(DOOM_KEY_LEFT_ARROW);
        doom_key_up(DOOM_KEY_RIGHT_ARROW);
        doom_key_up(DOOM_KEY_1);
        doom_key_up(DOOM_KEY_2);
        doom_key_up(DOOM_KEY_3);
        doom_key_up(DOOM_KEY_4);
        doom_key_up(DOOM_KEY_5);
        doom_key_up(DOOM_KEY_6);
        doom_key_up(DOOM_KEY_7);
    }

    if (crankChange > 5) {
        doom_key_down(DOOM_KEY_CTRL);
    }
    if (crankChange < -5) {
        cycleWeapons(players[0].readyweapon);
    }

    if ((current & kButtonA) | (pushed & kButtonA)) {
        doom_key_down(DOOM_KEY_ENTER);
    }
    if ((current & kButtonB) | (pushed & kButtonB)) {
        doom_key_down(DOOM_KEY_SPACE);
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
    }
    if ((released & kButtonB)) {
        doom_key_up(DOOM_KEY_SPACE);
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

static void cycleWeapons(weapontype_t currentWeapon) {
    weapontype_t nextWeapon = wp_nochange;
    ammotype_t nextAmmotype = am_noammo;
    doom_key_t nextWeaponKey = DOOM_KEY_UNKNOWN;

    switch (currentWeapon) {
        case wp_fist:
        case wp_chainsaw:
            nextWeapon = wp_pistol;
            nextAmmotype = am_clip;
            nextWeaponKey = DOOM_KEY_2;
            break;
        case wp_pistol:
            nextWeapon = wp_shotgun;
            nextAmmotype = am_shell;
            nextWeaponKey = DOOM_KEY_3;
            break;
        case wp_shotgun:
            nextWeapon = wp_chaingun;
            nextAmmotype = am_clip;
            nextWeaponKey = DOOM_KEY_4;
            break;
        case wp_chaingun:
            nextWeapon = wp_missile;
            nextAmmotype = am_misl;
            nextWeaponKey = DOOM_KEY_5;
            break;
        case wp_missile:
            nextWeapon = wp_plasma;
            nextAmmotype = am_cell;
            nextWeaponKey = DOOM_KEY_6;
            break;
        case wp_plasma:
            nextWeapon = wp_bfg;
            nextAmmotype = am_cell;
            nextWeaponKey = DOOM_KEY_7;
            break;
        case wp_bfg:
            nextWeapon = wp_fist;
            nextAmmotype = am_noammo;
            nextWeaponKey = DOOM_KEY_1;
            break;
    }

    if (!players[0].weaponowned[nextWeapon] || players[0].ammo[nextAmmotype] == 0) {
        // cannot equip the next weapon, try subsequent ones recursively
        cycleWeapons(nextWeapon);
        return;
    }

    doom_key_down(nextWeaponKey);
}
