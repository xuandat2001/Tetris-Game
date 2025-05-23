#pragma once
#include <stdbool.h>  // enable bool type
#ifndef GAME_STATE_H
#define GAME_STATE_H

/*typedef enum uint8_t {
    STATE_WELCOME,
    STATE_WAIT_FOR_START,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_HIGH_SCORE
} GameState;*/

typedef enum {
    STATE_WELCOME,
    STATE_WAIT_FOR_START,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_HIGH_SCORE
} GameState;

extern GameState currentState;  // Declare global state variable
extern bool isPaused;           // Declare pause flag

//extern GameState currentState;

void SetState(GameState newState);
void UpdateGameState(void);

#endif
