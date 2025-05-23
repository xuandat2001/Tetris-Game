#include <stdio.h>
#include "gameHeader.h"
#include "gameLogic.h"
#include "NuMicro.h"
#include "gameState.h"

#include "../EBI_LCD_Module.h"

bool isPaused = false; 

GameState currentState = STATE_WELCOME;

void SetState(GameState newState) {
    currentState = newState;

    switch (newState) {
        case STATE_WELCOME:
            LCD_Picture(0, 0, 240, 320, WelcomeScreen);
            break;

        case STATE_WAIT_FOR_START:
            LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
            LCD_PutString(40, 150, (uint8_t *)"Press SW1 to Start", COLOR_WHITE, C_BLACK);
            break;
				
        case STATE_PLAY:
            LCD_Picture(0, 0, 240, 320, backgroundScreenPlay);
            Draw_Current_Shape(80, 40, COLOR_BORDER);
						Timer0_Start(); // Resume time counting
            break;

        case STATE_PAUSE:
            LCD_PutString(60, 150, (uint8_t *)"PAUSED", COLOR_YELLOW, C_BLACK);
						Timer0_Stop(); // Freeze time
            break;

        case STATE_GAME_OVER:
						LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
						LCD_PutString(40, 150, (uint8_t *)"GAME OVER", COLOR_RED, C_BLACK);
						LCD_PutString(40, 170, (uint8_t *)"Score: ", COLOR_WHITE, C_BLACK);
						char scoreStr[10];
						sprintf(scoreStr, "%d", score);
						LCD_PutString(100, 170, (uint8_t *)scoreStr, COLOR_YELLOW, C_BLACK);
						//Save_High_Score(score); // Implement Flash saving
						break;

        case STATE_HIGH_SCORE:
           // LCD_PutString(20, 150, (uint8_t *)"HIGH SCORES:", COLOR_GREEN, C_BLACK);
            // TODO: display high score values
            break;
    }
}

/*void UpdateGameState(void) {
    switch (currentState) {
        case STATE_WELCOME:
            // Wait for SW1 to proceed to next state
            break;

        case STATE_WAIT_FOR_START:
            // Wait for SW1 to start the game
            break;

        case STATE_PLAY:
            // Handle block movement, drop timing, line clearing, etc.
            break;

        case STATE_PAUSE:
            // Freeze timers and input
            break;

        case STATE_GAME_OVER:
            // Wait for SW1 to go to high score or restart
            break;

        case STATE_HIGH_SCORE:
            // Wait for SW1 to return to PLAY
            break;
    }
}*/
