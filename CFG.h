#pragma once

// if true, LCD is rotated 180 and encoder direction is reversed
#define CFG_ROTATE_LCD      0

// if true, phaser is all the segments on the row, otherwise more sparse, with the "tribar"
#define CFG_PHASER_FULL     0

// if true, phaser row wraps around, otherwise pegged at top & bottom
#define CFG_PHASER_WRAP     0

// if true, lowercase characters are defined and used, otherwise all caps
#define CFG_LOWERCASE       0

// if true, phaser pulse is animated (does impact responsiveness)
#define CFG_ANIMATE_PHASER  0

// With each wave/level, the time between alien groups (characters) is reduced by NUM/DEN
#define CFG_WAVE_SPEED_NUM  3
#define CFG_WAVE_SPEED_DEN  4

// the time between alien groups at wave/level
#define CFG_WAVE_SPEED_MAX_MS 2000UL

// the minimum time between alien groups
#define CFG_WAVE_SPEED_MIN_MS 100UL

// with each wave/level, the number of alien groups is reduced by this:
#define CFG_WAVE_COUNT_DEC  1

// this is the minimum number of alien groups in a level/wave
#define CFG_WAVE_COUNT_MIN  5

// if true, serial output, no random seed
#define CFG_DEBUG           0

// if true, goes straight to a frozen view of gameplay
#define CFG_PHOTO           0
