/******************************************************************************
 * @file     gameState.h
 * @brief    Prototype states of Tetris game.
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
typedef enum {
    WELCOME_SCREEN,
    READY_SCREEN,
    PLAYING,
    PAUSED,
    GAME_OVER,
    HIGH_SCORE
} SystemState;

extern volatile SystemState currentState;