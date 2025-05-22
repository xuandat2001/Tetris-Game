#ifndef GAME_STATE_H
#define GAME_STATE_H

typedef enum uint8_t {
    STATE_WELCOME,
    STATE_WAIT_FOR_START,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_HIGH_SCORE
} GameState;

extern GameState currentState;

void SetState(GameState newState);
void UpdateGameState(void);

#endif
