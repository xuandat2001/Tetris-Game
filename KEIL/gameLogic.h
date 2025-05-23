#ifndef GAME_LOGIC_H
#pragma once
#include <stdint.h> 
//typedef enum { false, true } bool;
#include <stdbool.h>  
#define SQUARE_SIZE 10
#define COLOR_BORDER 0x0000 // Black
#define GRID_WIDTH 14
#define GRID_HEIGHT 24
extern bool isPaused;
extern int nextShapeIndex; // Holds next shape to appear
extern int x, y; // Declare as extern to share across files

// === Block and Tetromino ===
typedef struct {
    int x, y;
} Block;

typedef struct {
    Block blocks[4];   // A tetromino has 4 blocks
    uint16_t color;    // 16-bit RGB565 color
} Tetromino;

extern Tetromino currentShape;
extern int currentX, currentY;

// === All 7 Tetromino Shapes ===
extern Tetromino shapes[7];

// === Function Prototypes ===
void Rotate_Clockwise(Tetromino* shape);
void Draw_Tetromino(uint16_t originX, uint16_t originY, Tetromino shape, uint16_t borderColor);


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
void Draw_Current_Shape(int x, int y, uint16_t borderColor);
void Test_Draw_All_Shapes(void);
void Reset_Game();  // Declare reset function

extern uint8_t gameGrid[GRID_HEIGHT][GRID_WIDTH];
extern Tetromino currentShape;
extern int currentX, currentY;
extern int currentRotation;
extern int score;
extern int level;

void Rotate_Clockwise(Tetromino* shape);
bool Check_Collision(int newX, int newY, Tetromino *shape);
void Merge_Shape_Into_Grid();
int Clear_Lines();
void Update_Level_Speed();
void Spawn_New_Shape();
void Draw_Grid(void);

void Rotate_Current_Block(void);
void Move_Block_Left(void);
void Move_Block_Right(void);
void Drop_Block_Fast(void);
#endif
