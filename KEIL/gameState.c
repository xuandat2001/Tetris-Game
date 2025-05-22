#include <stdio.h>
#include "gameHeader.h"
#include "gameLogic.h"
#include "NuMicro.h"
#include "gameState.h"

GameState currentState = STATE_WELCOME;

void SetState(GameState newState) {
    currentState = newState;

    switch (newState) {
        case STATE_WELCOME:
            LCD_Picture(0, 0, 240, 320, WelcomeScreen);
            break;

        case STATE_WAIT_FOR_START:
            LCD_PutString(40, 150, (uint8_t *)"Press SW1 to Start", COLOR_WHITE, C_BLACK);
            break;

        case STATE_PLAY:
            LCD_Picture(0, 0, 240, 320, backgroundScreenPlay);
            Draw_Current_Shape(80, 40, COLOR_BORDER);
            break;

        case STATE_PAUSE:
            LCD_PutString(60, 150, (uint8_t *)"PAUSED", COLOR_YELLOW, C_BLACK);
            break;

        case STATE_GAME_OVER:
            LCD_PutString(40, 150, (uint8_t *)"GAME OVER", COLOR_RED, C_BLACK);
            break;

        case STATE_HIGH_SCORE:
            LCD_PutString(20, 150, (uint8_t *)"HIGH SCORES:", COLOR_GREEN, C_BLACK);
            // TODO: display high score values
            break;
    }
}
