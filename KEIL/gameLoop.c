/******************************************************************************
 * @file     gameLoop.c
 * @brief    This file is to place draw functions, the game loop of the PLAYING state.
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NuMicro.h"
#include <math.h>

#include "gameHeader.h"
#include "../EBI_LCD_Module.h"
#include "gameLogic.h"
#include "gameState.h"
int isPaused;
int needsRedraw = 1;
Tetromino currentTetromino;
Tetromino nextTetromino;
int nextShapeIndex = -1;
int currentShapeIndex = -1;
int currentX, currentY;
int isPaused = 0;
// Draw a Picture
void LCD_Picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *img)
{
    uint32_t i;
    LCD_SetWindow(x, x + width - 1, y, y + height - 1);
    for (i = 0; i < width * height; i++) {
        LCD_WR_DATA(img[i]);
    }
}


void LCD_Draw_FilledSquare(uint16_t x, uint16_t y, uint16_t fillColor, uint16_t borderColor) {
    
		uint16_t i;
    // Draw full 10x10 square in border color
    LCD_SetWindow(x, x + SQUARE_SIZE - 1, y, y + SQUARE_SIZE - 1);
    for (i = 0; i < SQUARE_SIZE * SQUARE_SIZE; i++) {
        LCD_WR_DATA(borderColor);
    }

    // Draw smaller 8x8 filled square inside (1-pixel margin)
    LCD_SetWindow(x + 1, x + SQUARE_SIZE - 2, y + 1, y + SQUARE_SIZE - 2);
    for (i = 0; i < 8 * 8; i++) {
        LCD_WR_DATA(fillColor);
    }
}
void LCD_Draw_SquareBorder(uint16_t x, uint16_t y, uint16_t rows, uint16_t cols, uint16_t fillColor, uint16_t borderColor) {
    
    uint16_t i;

    // Top row
    for (i = 0; i < cols; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y, fillColor, borderColor);
    }

    // Bottom row
    for (i = 0; i < cols; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y + (rows - 1) * SQUARE_SIZE, fillColor, borderColor);
    }

    // Left column (excluding corners)
    for (i = 1; i < rows - 1; i++) {
        LCD_Draw_FilledSquare(x, y + i * SQUARE_SIZE, fillColor, borderColor);
    }

    // Right column (excluding corners)
    for (i = 1; i < rows - 1; i++) {
        LCD_Draw_FilledSquare(x + (cols - 1) * SQUARE_SIZE, y + i * SQUARE_SIZE, fillColor, borderColor);
    }
}
void LCD_Draw_SquareRow(uint16_t x, uint16_t y, uint16_t cols, uint16_t fillColor, uint16_t borderColor) {
		uint16_t i;
    for (i = 0; i < cols; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y, fillColor, borderColor);
    }
}
void LCD_Draw_SquareColumn(uint16_t x, uint16_t y, uint16_t rows, uint16_t fillColor, uint16_t borderColor) {
		uint16_t i;
    for (i = 0; i < rows; i++) {
        LCD_Draw_FilledSquare(x, y + i * SQUARE_SIZE, fillColor, borderColor);
    }
}


void Draw_I_Shape(uint16_t x, uint16_t y, uint16_t borderColor) {
	int i;
    for (i = 0; i < 4; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y, COLOR_CYAN, borderColor);
    }
}
void Draw_J_Shape(uint16_t x, uint16_t y,  uint16_t borderColor) {
    LCD_Draw_FilledSquare(x, y, COLOR_BLUE, borderColor);
		int i;
    for (i = 0; i < 3; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y + SQUARE_SIZE, COLOR_BLUE, borderColor);
    }
}
void Draw_L_Shape(uint16_t x, uint16_t y,  uint16_t borderColor) {
		int i;
    for (i = 0; i < 3; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y + SQUARE_SIZE, COLOR_ORANGE, borderColor);
    }
    LCD_Draw_FilledSquare(x + 2 * SQUARE_SIZE, y, COLOR_ORANGE, borderColor);
}

void Draw_S_Shape(uint16_t x, uint16_t y,  uint16_t borderColor) {
		LCD_Draw_FilledSquare(x + SQUARE_SIZE, y, COLOR_GREEN, borderColor);
    LCD_Draw_FilledSquare(x + 2 * SQUARE_SIZE, y, COLOR_GREEN, borderColor);
    LCD_Draw_FilledSquare(x, y + SQUARE_SIZE, COLOR_GREEN, borderColor);
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y + SQUARE_SIZE, COLOR_GREEN, borderColor);
}
void Draw_O_Shape(uint16_t x, uint16_t y,  uint16_t borderColor) {
    LCD_Draw_FilledSquare(x, y, COLOR_YELLOW, borderColor);
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y, COLOR_YELLOW, borderColor);
    LCD_Draw_FilledSquare(x, y + SQUARE_SIZE, COLOR_YELLOW, borderColor);
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y + SQUARE_SIZE, COLOR_YELLOW, borderColor);
}
void Draw_T_Shape(uint16_t x, uint16_t y,  uint16_t borderColor) {
		int i;
    for (i = 0; i < 3; i++) {
        LCD_Draw_FilledSquare(x + i * SQUARE_SIZE, y, COLOR_PURPLE, borderColor);
    }
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y + SQUARE_SIZE, COLOR_PURPLE, borderColor);
}
void Draw_Z_Shape(uint16_t x, uint16_t y, uint16_t borderColor) {
    LCD_Draw_FilledSquare(x, y, COLOR_RED, borderColor);
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y, COLOR_RED, borderColor);
    LCD_Draw_FilledSquare(x + SQUARE_SIZE, y + SQUARE_SIZE, COLOR_RED, borderColor);
    LCD_Draw_FilledSquare(x + 2 * SQUARE_SIZE, y + SQUARE_SIZE, COLOR_RED, borderColor);
}

void Draw_Sidebar_UI(void) {
    uint16_t sidebarX = 175;  // Grid width + 1-block padding = 120 px
		uint16_t sidebarX1 = 165;
    uint16_t lineY;
    // Title: "NEXT"
    LCD_PutString(sidebarX, 25, (uint8_t *)"NEXT", COLOR_WHITE, 0x2104);
    // "SCORE"
    lineY = 95;
    LCD_PutString(sidebarX, lineY, (uint8_t *)"SCORE", COLOR_WHITE, 0x2104);

    // "TIME"
    lineY = 145;
    LCD_PutString(sidebarX, lineY, (uint8_t *)"TIME", COLOR_WHITE, 0x2104);

    // "LEVEL"
    lineY = 195;
	LCD_PutString(sidebarX, lineY, (uint8_t *)"LEVEL", COLOR_WHITE, 0x2104);

 
    // "RESTART"
    lineY = 260;
    LCD_PutString(sidebarX1, lineY, (uint8_t *)"RESTART", COLOR_WHITE, 0x2104);

    // "PAUSE"
    lineY = 280;
    LCD_PutString(sidebarX1, lineY, (uint8_t *)"PAUSE", COLOR_WHITE, 0x2104);
}
void LCD_Draw_Border(){
	LCD_Picture(0, 0, 240, 320, backgroundScreenPlay);
	LCD_Draw_SquareBorder(0, 0, 32, 24,  0x8922, 0x0000);  // light gray border
	// Draw vertical grey column (20 height between play area and side panel)
	LCD_Draw_SquareColumn(15 * SQUARE_SIZE, 0, 31, 0x8922, 0x0000);  // x=15 since play area is 14 + 1 spacing

	// Draw top horizontal grey row (under NEXT)
	LCD_Draw_SquareRow(15 * SQUARE_SIZE, 7 * SQUARE_SIZE, 8, 0x8922, 0x0000);  // 7 squares wide

	// Draw bottom horizontal grey row (above RESTART)
	LCD_Draw_SquareRow(15 * SQUARE_SIZE, 24 * SQUARE_SIZE, 8, 0x8922, 0x0000);  // 7 squares wide  	
	Draw_Sidebar_UI();
}

/* Timer display */
void LCD_DisplayTime(uint8_t min, uint8_t sec)
{
		uint16_t sidebarX = 175;  
		uint16_t sidebarX1 = 165;
    uint16_t lineY;
    char buffer[6];
    sprintf(buffer, "%02d:%02d", min, sec);
		
		lineY = 165;
    // Draw updated time at a clearly visible place

    LCD_PutString(sidebarX, lineY, (uint8_t *)buffer, COLOR_WHITE, 0x2104);     // Show time
}

void Draw_Score_Level(void) {
    uint16_t sidebarX = 175;  
    char buffer[16];          

    // Draw Score
    sprintf(buffer, "%lu", score); 
    LCD_PutString(sidebarX, 115, (uint8_t *)buffer, COLOR_WHITE, 0x2104);

    // Draw Level
    sprintf(buffer, "%u", level);
    LCD_PutString(sidebarX, 215, (uint8_t *)buffer, COLOR_WHITE, 0x2104); 
}


void gameLoop() {
    static uint8_t initialized = 0;
    displayTime();

    if (currentState == PLAYING) { // Only run when in PLAYING state
        if (!initialized) {
            LCD_Draw_Border(); // Draw game UI once
            initialized = 1;
        }

        if (needsRedraw && !isPaused) {
            needsRedraw = 0;
            DrawBoard();
            Draw_Score_Level();
            Draw_Tetromino(currentTetromino, 0x0000);
            Draw_next_Shape(175, 45, 0x0000, &nextTetromino);
        }
    } else {
        initialized = 0; // Reset UI when leaving PLAYING state
    }
}