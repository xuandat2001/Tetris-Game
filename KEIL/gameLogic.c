/******************************************************************************
 * @file     gamelogic.c
 * @brief    This file is to place  logic functions of Tetris game.
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NuMicro.h"
#include <math.h>
#include "gameLogic.h"
#include "gameState.h"
#include "gameHeader.h"
#include "../EBI_LCD_Module.h"
uint32_t highScores[MAX_HIGH_SCORES] = {0};
volatile SystemState currentState = WELCOME_SCREEN; // Actual definition
// Shape Rotation 
// All shapes in default orientation (rotation 0)
const uint32_t level_cmp[11] = {
    0, // unused, index starts at 1
    5999999, // Level 1: 0.5s 
    5399999, // Level 2: 0.45s
    4799999, // Level 3: 0.4s
    4199999, // Level 4: 0.35s
    3599999, // Level 5: 0.3s
    2999999, // Level 6: 0.25s
    2399999, // Level 7: 0.2s
    1799999, // Level 8: 0.15s
    1199999, // Level 9: 0.1s
    599999   // Level 10: 0.05s
};
uint32_t score = 0;
uint8_t level = 1;
extern volatile uint8_t Timer0_flag;
extern volatile uint8_t timer_running;
extern uint8_t seconds;
extern uint8_t minutes;
int board[BOARD_HEIGHT_2D][BOARD_WIDTH_2D] = {0};

Tetromino shapes[7] = {
    {{{0,0}, {1,0}, {2,0}, {3,0}}, COLOR_CYAN},     // I
    {{{0,0}, {0,1}, {1,1}, {2,1}}, COLOR_BLUE},     // J
    {{{0,1}, {1,1}, {2,1}, {2,0}}, COLOR_ORANGE},   // L
    {{{0,0}, {1,0}, {0,1}, {1,1}}, COLOR_YELLOW},   // O (no rotation)
    {{{1,0}, {2,0}, {0,1}, {1,1}}, COLOR_GREEN},    // S
    {{{0,0}, {1,0}, {2,0}, {1,1}}, COLOR_PURPLE},   // T
    {{{0,0}, {1,0}, {1,1}, {2,1}}, COLOR_RED}       // Z
};

void Rotate_Clockwise(Tetromino* shape) {
    Tetromino original = *shape;
	int i;
    for (i = 0; i < 4; i++) {
        int x = shape->blocks[i].x - original.blocks[1].x; // Pivot around second block
        int y = shape->blocks[i].y - original.blocks[1].y;
        shape->blocks[i].x = original.blocks[1].x - y;
        shape->blocks[i].y = original.blocks[1].y + x;
    }
    if (Check_Collision(shape)) {
        *shape = original; // Revert if collision
    }
}




ShapeDrawFunc shapeFunctions[] = {
    Draw_I_Shape,
    Draw_J_Shape,
    Draw_L_Shape,
    Draw_O_Shape,
    Draw_S_Shape,
    Draw_T_Shape,
    Draw_Z_Shape
};

const char* shapeNames[] = {
    "I", "J", "L", "O", "S", "T", "Z"
};
void Draw_Tetromino(Tetromino shape, uint16_t borderColor) {
	int i;
    for (i = 0; i < 4; i++) {
        LCD_Draw_FilledSquare(shape.blocks[i].x, shape.blocks[i].y, shape.color, borderColor);
    }
}

void Prepare_Next_Shape() {
    nextShapeIndex = rand() % 7;
}

Tetromino CreateTetromino(int shapeIndex, uint16_t pixelX, uint16_t pixelY, uint16_t color) {
    Tetromino t;
    t.color = color;
    
    switch (shapeIndex) {
        case 0: // I-shape (horizontal)
            t.blocks[0].x = pixelX + 0*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 1*SQUARE_SIZE; t.blocks[1].y = pixelY;
            t.blocks[2].x = pixelX + 2*SQUARE_SIZE; t.blocks[2].y = pixelY;
            t.blocks[3].x = pixelX + 3*SQUARE_SIZE; t.blocks[3].y = pixelY;
            break;
            
        case 1: // J-shape
            t.blocks[0].x = pixelX + 0*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 0*SQUARE_SIZE; t.blocks[1].y = pixelY + SQUARE_SIZE;
            t.blocks[2].x = pixelX + 1*SQUARE_SIZE; t.blocks[2].y = pixelY + SQUARE_SIZE;
            t.blocks[3].x = pixelX + 2*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
            
        case 2: // L-shape
            t.blocks[0].x = pixelX + 2*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 0*SQUARE_SIZE; t.blocks[1].y = pixelY + SQUARE_SIZE;
            t.blocks[2].x = pixelX + 1*SQUARE_SIZE; t.blocks[2].y = pixelY + SQUARE_SIZE;
            t.blocks[3].x = pixelX + 2*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
            
        case 3: // O-shape
            t.blocks[0].x = pixelX + 0*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 1*SQUARE_SIZE; t.blocks[1].y = pixelY;
            t.blocks[2].x = pixelX + 0*SQUARE_SIZE; t.blocks[2].y = pixelY + SQUARE_SIZE;
            t.blocks[3].x = pixelX + 1*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
            
        case 4: // S-shape
            t.blocks[0].x = pixelX + 1*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 2*SQUARE_SIZE; t.blocks[1].y = pixelY;
            t.blocks[2].x = pixelX + 0*SQUARE_SIZE; t.blocks[2].y = pixelY + SQUARE_SIZE;
            t.blocks[3].x = pixelX + 1*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
            
        case 5: // T-shape
            t.blocks[0].x = pixelX + 0*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 1*SQUARE_SIZE; t.blocks[1].y = pixelY;
            t.blocks[2].x = pixelX + 2*SQUARE_SIZE; t.blocks[2].y = pixelY;
            t.blocks[3].x = pixelX + 1*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
            
        case 6: // Z-shape
            t.blocks[0].x = pixelX + 0*SQUARE_SIZE; t.blocks[0].y = pixelY;
            t.blocks[1].x = pixelX + 1*SQUARE_SIZE; t.blocks[1].y = pixelY;
            t.blocks[2].x = pixelX + 1*SQUARE_SIZE; t.blocks[2].y = pixelY + SQUARE_SIZE;
            t.blocks[3].x = pixelX + 2*SQUARE_SIZE; t.blocks[3].y = pixelY + SQUARE_SIZE;
            break;
    }
    
    return t;
}

uint16_t GetShapeColor(int shapeIndex) {
    switch(shapeIndex) {
        case 0: return COLOR_CYAN;
        case 1: return COLOR_BLUE;
        case 2: return COLOR_ORANGE;
        case 3: return COLOR_YELLOW;
        case 4: return COLOR_GREEN;
        case 5: return COLOR_PURPLE;
        case 6: return COLOR_RED;
				case 7: return BACKGROUND_COLOR;
        default: return 0xFFFF; //  default color
    }
}

void Promote_Next_Shape() {
    currentShapeIndex = nextShapeIndex;
    Prepare_Next_Shape(); // Generate new next shape
}
void Erase_Current_Shape(int x, int y, uint16_t borderColor, Tetromino* currentTetromino) {
    *currentTetromino = CreateTetromino(currentShapeIndex, x, y, BACKGROUND_COLOR);
    Draw_Tetromino(*currentTetromino, BACKGROUND_COLOR);
}
void Draw_Current_Shape(int x, int y, uint16_t borderColor, Tetromino* currentTetromino) {
    *currentTetromino = CreateTetromino(currentShapeIndex, x, y, GetShapeColor(currentShapeIndex));
    Draw_Tetromino(*currentTetromino, borderColor);
}

void Draw_next_Shape(int x, int y, uint16_t borderColor, Tetromino* nextTetromino) {
    *nextTetromino = CreateTetromino(nextShapeIndex, x, y, GetShapeColor(nextShapeIndex));
    Draw_Tetromino(*nextTetromino, borderColor);
}
void Erase_next_Shape(int x, int y, uint16_t borderColor, Tetromino* nextTetromino) {
    *nextTetromino = CreateTetromino(nextShapeIndex, x, y, BACKGROUND_COLOR);
    Draw_Tetromino(*nextTetromino, BACKGROUND_COLOR);
}
void Erase_Current_Shape_Rotate(int x, int y, uint16_t borderColor, Tetromino* currentTetromino) {
    uint16_t originalColor = currentTetromino->color;
    currentTetromino->color = BACKGROUND_COLOR;
    Draw_Tetromino(*currentTetromino, BACKGROUND_COLOR);
    currentTetromino->color = originalColor;
}

void Draw_Current_Shape_Rotate(int x, int y, uint16_t borderColor, Tetromino* currentTetromino) {
    Draw_Tetromino(*currentTetromino, borderColor);
}

// Shape movement
void Timer1_SetIntervalByLevel(uint8_t level)
{
    if (level < 1) level = 1;
    if (level > 10) level = 10;

    TIMER1->CMP = level_cmp[level];
}
void UpdateScore(uint32_t points) {
    score += points;
    uint8_t newLevel = (score / POINTS_PER_LEVEL) + 1;
    if (newLevel > MAX_LEVEL) newLevel = MAX_LEVEL;
    if (newLevel != level) {
        level = newLevel;
        Timer1_SetIntervalByLevel(level); 
        needsRedraw = 1; // Force UI update for new level
    }
    needsRedraw = 1; // Force UI update for new score
}


void SpawnNewTetromino() {
    // Promote the next shape to current
    currentShapeIndex = nextShapeIndex;
    
    // Create new shape at top center
    currentTetromino = CreateTetromino(currentShapeIndex, 
                                     SPAWN_X ,  // Center horizontally
                                     SPAWN_Y ,                 // Top of grid
                                     GetShapeColor(currentShapeIndex));
    
    // Generate new next shape
    Prepare_Next_Shape();
}
int Check_Collision(Tetromino *t) {
    int i;
    for (i = 0; i < 4; i++) {
        int gridX = t->blocks[i].x / SQUARE_SIZE;
        int gridY = t->blocks[i].y / SQUARE_SIZE;

        // Check boundaries (updated conditions)
        if (gridX <= 0 || gridX >= BOARD_WIDTH_2D+1|| gridY + 1 >= BOARD_HEIGHT_2D)
            return 1;
        // Check if cell is occupied (only if within vertical bounds)
        if (gridY >= 0 && board[gridY][gridX] != 0)
            return 1;
    }
    return 0;
}
void Lock_Tetromino_To_Board(Tetromino* t) {
	int i;
    for (i = 0; i < 4; i++) {
        int gridX = t->blocks[i].x / SQUARE_SIZE;
        int gridY = t->blocks[i].y / SQUARE_SIZE;
        if (gridY >= 0 && gridY <= BOARD_HEIGHT_2D && gridX >= 0 && gridX <= BOARD_WIDTH_2D) {
            board[gridY][gridX] = t->color;
        }
    }
}
int CheckAndClearLines() {
    int row;
    int col;
    int r;
    int linesCleared = 0;
    for (row = BOARD_HEIGHT_2D - 2; row >= 1; row--) {
        int isFull = 1;
        // Check all columns in the row (0 to BOARD_WIDTH_2D - 1)
        for (col = 0; col < BOARD_WIDTH_2D; col++) {
            if (board[row][col] == 0) {
                isFull = 0;
                break;
            }
        }
        if (isFull) {
            // Shift rows above down by 1
            for (r = row; r > 0; r--) {
                memcpy(board[r], board[r - 1], sizeof(board[r]));
            }
            // Clear the top row
            memset(board[0], 0, sizeof(board[0]));
            linesCleared++;
            ++row; // Re-check the same row index after shift
        }
    }
    return linesCleared;
}
void MoveTetrominoDown() {
    printf("Current Position (Before Move): X=%d, Y=%d\n", currentX, currentY);

    // Create a temporary tetromino by copying the CURRENT (possibly rotated) tetromino
    Tetromino temp = currentTetromino;
    int i;

    // Move all blocks of the temporary tetromino down by SQUARE_SIZE
    for (i = 0; i < 4; i++) {
        temp.blocks[i].y += SQUARE_SIZE;
    }

    // Check collision with the TEMPORARY (rotated) tetromino
    if (!Check_Collision(&temp)) {
        // Erase current position
        Erase_Current_Shape_Rotate(currentX, currentY, BACKGROUND_COLOR, &currentTetromino);
        
        // Update Y position
        currentY += SQUARE_SIZE;
        
        // Update the actual currentTetromino's position
        currentTetromino = temp; //  Preserve rotation state
        
        // Redraw at new position
        Draw_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
    } else {
        // Handle collision (lock tetromino, check lines, etc.)
        Lock_Tetromino_To_Board(&currentTetromino);
        int linesCleared = CheckAndClearLines();
        UpdateScore(linesCleared);
        // Game Over Check (top 2 rows)
        int gameOver = 0;
				int row, col;
        for (row = 0; row <= BOARD_HEIGHT_2D; row++) {
            for (col = 0; col <= BOARD_WIDTH_2D; col++) {
                if (board[1][col] != 0 && board[4][col] != 0) {
                    gameOver = 1;
                    break;
                }
								else{
									continue;
								}
            }
            if (gameOver) break;
        }
        if (gameOver) {
            currentState = GAME_OVER;  // Transition to game-over state
            TIMER0->CTL &= ~(1 << 30); // Stop timers
            TIMER1->CTL &= ~(1 << 30);
        } else {
            // Normal shape promotion
            Erase_next_Shape(175, 45, 0x0000, &nextTetromino);
            Promote_Next_Shape();
            currentX = SPAWN_X;
            currentY = SPAWN_Y;
            currentTetromino = CreateTetromino(currentShapeIndex, currentX, currentY, 
                                             GetShapeColor(currentShapeIndex));
        }
    }
}

void DrawBoard() {
	int row;
	int col;
	   LCD_BlankArea(
        SQUARE_SIZE, SQUARE_SIZE, 
        BOARD_WIDTH_2D * SQUARE_SIZE, 
        (BOARD_HEIGHT_2D * SQUARE_SIZE) - 20 , 
        C_BLACK
    );
    for (row = 1; row < BOARD_HEIGHT_2D; row++) {
        for (col = 1; col < BOARD_WIDTH_2D+1; col++) {
            if (board[row][col] != 0) {
                uint16_t x = col * SQUARE_SIZE;
                uint16_t y = row * SQUARE_SIZE;
                LCD_Draw_FilledSquare(x, y, board[row][col], COLOR_BORDER);
            }
        }
    }
}


void displayTime(void)
{
	  if (Timer0_flag == 1)
    {
        Timer0_flag = 0;

        if (timer_running)
        {
            seconds++;
            if (seconds >= 60)
            {
                seconds = 0;
                minutes++;
            }

            LCD_DisplayTime(minutes, seconds);
        }
    }
}


void HardDrop() {
    while (1) {
        // Create a temporary copy of the CURRENT (rotated) tetromino
        Tetromino temp = currentTetromino;
        int i;
        // Move all blocks down
        for (i = 0; i < 4; i++) {
            temp.blocks[i].y += SQUARE_SIZE;
        }
        // Check collision with the rotated shape
        if (Check_Collision(&temp)) break;
        // Erase using rotated state
        Erase_Current_Shape_Rotate(currentX, currentY, BACKGROUND_COLOR, &currentTetromino);
        currentY += SQUARE_SIZE;
        currentTetromino = temp; // Preserve rotation
        // Draw using rotated state
        Draw_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
    }
    Lock_Tetromino_To_Board(&currentTetromino);
    Erase_next_Shape(175, 45, 0x0000, &nextTetromino);
    Promote_Next_Shape();
    currentX = SPAWN_X;
    currentY = SPAWN_Y;
    currentTetromino = CreateTetromino(currentShapeIndex, currentX, currentY, GetShapeColor(currentShapeIndex));
    needsRedraw = 1;
}
void MoveTetrominoLeft() {
    Tetromino temp = currentTetromino;
    int i;
    for (i = 0; i < 4; i++) {
        temp.blocks[i].x -= SQUARE_SIZE;
    }
    if (!Check_Collision(&temp)) {
        Erase_Current_Shape_Rotate(currentX, currentY, BACKGROUND_COLOR, &currentTetromino);
        currentX -= SQUARE_SIZE;
        currentTetromino = temp; // Update the currentTetromino's position
        Draw_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
        needsRedraw = 1;
    }
}

void MoveTetrominoRight() {
     Tetromino temp = currentTetromino;
    int i;
    for (i = 0; i < 4; i++) {
        temp.blocks[i].x += SQUARE_SIZE;
    }
    if (!Check_Collision(&temp)) {
        Erase_Current_Shape_Rotate(currentX, currentY, BACKGROUND_COLOR, &currentTetromino);
        currentX += SQUARE_SIZE;
        currentTetromino = temp; // Update the currentTetromino's position
        Draw_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
        needsRedraw = 1;
    }
}
void SaveHighScore(uint32_t newScore) {
	int i; 
	int j;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        if (newScore > highScores[i]) {
            for (j = MAX_HIGH_SCORES - 1; j > i; j--) {
                highScores[j] = highScores[j - 1];
            }
            highScores[i] = newScore;
            break;
        }
    }
}

void DisplayHighScores(void) {
    LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
    LCD_PutString(50, 50, (uint8_t *)"HIGH SCORES", C_WHITE, C_BLACK);
    char buffer[32];
    int y = 80;
		int i;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        sprintf(buffer, "%d. %u", i + 1, highScores[i]);
        LCD_PutString(50, y, (uint8_t *)buffer, C_WHITE, C_BLACK);
        y += 20;
    }
}
