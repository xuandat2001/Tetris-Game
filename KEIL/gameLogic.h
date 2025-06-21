/******************************************************************************
 * @file     gamelogic.h
 * @brief    Prototype of gameLogic.
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <stdint.h> 
// Grid dimensions (blocks)
// Spawn position (centered at top)
#define SPAWN_X   60  // unit(10x10 for each unit)
#define SPAWN_Y   10 // unit(10x10 for each unit)
#define MAX_HIGH_SCORES 5
#define BACKGROUND_COLOR    0x10a2  // I shape
extern int nextShapeIndex; // Holds next shape to appear
extern int currentShapeIndex; // Holds next shape to appear
extern int currentY;
extern int currentX;

#define BOARD_WIDTH_2D 14
#define BOARD_HEIGHT_2D 32 // exclude 2 unit for top and down border
extern int board[BOARD_HEIGHT_2D][BOARD_WIDTH_2D]; // 0 = empty, 1 = filled
#define POINTS_PER_LEVEL 5
#define MAX_LEVEL 10
extern uint32_t score;  // Current score
extern uint8_t level;   // Current level
extern uint32_t highScores[MAX_HIGH_SCORES];
void SaveHighScore(uint32_t newScore);
void DisplayHighScores(void);


// === Block and Tetromino ===
typedef struct {
    int x, y;
} Block;

typedef struct {
    Block blocks[4];   // A tetromino has 4 blocks
    uint16_t color;    // 16-bit RGB565 color
} Tetromino;
extern Tetromino currentTetromino;
extern Tetromino nextTetromino;
// === All 7 Tetromino Shapes ===
extern Tetromino shapes[7];

// === Function Prototypes ===
void Rotate_Clockwise(Tetromino* shape);
void Draw_Tetromino(Tetromino shape, uint16_t borderColor);


// === Typedef ===
typedef void (*ShapeDrawFunc)(uint16_t x, uint16_t y, uint16_t borderColor);

// === Shape Draw Functions ===
void Draw_I_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_J_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_L_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_O_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_S_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_T_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_Z_Shape(uint16_t x, uint16_t y, uint16_t borderColor);

// === External Arrays ===
extern ShapeDrawFunc shapeFunctions[7];
extern const char* shapeNames[7];

// === Game Shape Controls ===
void Prepare_Next_Shape(void);
void Promote_Next_Shape();
void Draw_Current_Shape(int x, int y, uint16_t borderColor, Tetromino* currentTetromino );
void Draw_next_Shape(int x, int y, uint16_t borderColor, Tetromino* nextTetromino);
void Test_Draw_All_Shapes(void);
void MoveTetrominoDown();
void Timer1_SetIntervalByLevel(uint8_t level);
void UpdateScore(uint32_t points);
void displayTime(void);
Tetromino CreateTetromino(int shapeIndex, uint16_t pixelX, uint16_t pixelY, uint16_t color); //
void Draw_Current_Shape_Rotate(int x, int y, uint16_t borderColor, Tetromino* currentTetromino);
void Erase_Current_Shape_Rotate(int x, int y, uint16_t borderColor, Tetromino* currentTetromino);
#endif