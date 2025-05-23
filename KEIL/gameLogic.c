#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NuMicro.h"
#include <math.h>
#include "gameLogic.h"
#include "gameHeader.h"
#include "gameState.h"
#include "../EBI_LCD_Module.h"
int x, y; 
// Shape Rotation 
// All shapes in default orientation (rotation 0)
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
	int i;
    for (i = 0; i < 4; i++) {
        int x = shape->blocks[i].x;
        int y = shape->blocks[i].y;
        shape->blocks[i].x = y;
        shape->blocks[i].y = -x;
    }
}

void Draw_Tetromino(uint16_t originX, uint16_t originY, Tetromino shape, uint16_t borderColor) {
		int i;
    for (i = 0; i < 4; i++) {
        uint16_t drawX = originX + shape.blocks[i].x * SQUARE_SIZE;
        uint16_t drawY = originY + shape.blocks[i].y * SQUARE_SIZE;
        LCD_Draw_FilledSquare(drawX, drawY, shape.color, borderColor);
    }
}

/* Usage 
Tetromino current = shapes[nextShapeIndex]; // get base shape
Rotate_Clockwise(&current);                 // rotate as needed
Draw_Tetromino(START_X, START_Y, current, BORDER_COLOR); // draw*/

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
void Prepare_Next_Shape() {
    nextShapeIndex = rand() % 7;
}
void Draw_Current_Shape(int x, int y, uint16_t borderColor) {
    if (nextShapeIndex < 0 || nextShapeIndex > 6) {
        Prepare_Next_Shape(); // Fallback
    }

    // Draw the current shape
    shapeFunctions[nextShapeIndex](x, y, borderColor);
    
    // Log for debug
    printf("Current Shape: %s\n", shapeNames[nextShapeIndex]);

    // Prepare next shape
    Prepare_Next_Shape();
}

void Test_Draw_All_Shapes(void) {
    uint16_t borderColor = 0x0000; // Black border
    // Draw each shape at a different position
    Draw_I_Shape(10, 10, borderColor);       // Cyan
    Draw_J_Shape(10, 40, borderColor);       // Blue
    Draw_L_Shape(70, 40, borderColor);       // Orange
    Draw_O_Shape(130, 40, borderColor);      // Yellow
    Draw_S_Shape(10, 80, borderColor);       // Green
    Draw_T_Shape(70, 80, borderColor);       // Purple
    Draw_Z_Shape(130, 80, borderColor);      // Red
}

void LCD_DisplayTime(uint8_t min, uint8_t sec)
{
    char buffer[6];
    sprintf(buffer, "%02d:%02d", min, sec);

    // Draw updated time at a clearly visible place
    LCD_PutString(0, 200, (uint8_t *)"        ", C_BLACK, C_BLACK);  // Clear area
    LCD_PutString(0, 200, (uint8_t *)buffer, C_YELLOW, C_BLACK);     // Show time
}

#include "gameLogic.h"

uint8_t gameGrid[GRID_HEIGHT][GRID_WIDTH] = {0};
Tetromino currentShape;
int currentX, currentY, currentRotation;
int score = 0;
int level = 1;
int i;
int x;
int y;
int ny;
// Collision detection
bool Check_Collision(int newX, int newY, Tetromino *shape) {
    for (i = 0; i < 4; i++) {
        int gridX = newX + shape->blocks[i].x;
        int gridY = newY + shape->blocks[i].y;
        if (gridX < 0 || gridX >= GRID_WIDTH || gridY >= GRID_HEIGHT) return true;
        if (gridY >= 0 && gameGrid[gridY][gridX]) return true;
    }
    return false;
}

// Merge current shape into grid
void Merge_Shape_Into_Grid() {
    for (i = 0; i < 4; i++) {
        int x = currentX + currentShape.blocks[i].x;
        int y = currentY + currentShape.blocks[i].y;
        if (y >= 0) gameGrid[y][x] = currentShape.color;
    }
}

// Clear filled lines and return count
int Clear_Lines() {
    int linesCleared = 0;
    for (y = GRID_HEIGHT - 1; y >= 0; y--) {
        int filled = 1;
        for (x = 0; x < GRID_WIDTH; x++) {
            if (!gameGrid[y][x]) {
                filled = 0;
                break;
            }
        }
        if (filled) {
            // Shift all rows above down
            for (ny = y; ny > 0; ny--) {
                memcpy(gameGrid[ny], gameGrid[ny - 1], GRID_WIDTH);
            }
            memset(gameGrid[0], 0, GRID_WIDTH);
            linesCleared++;
            y++; // Recheck the same row
        }
    }
    return linesCleared;
}

// Update level and timer speed
void Update_Level_Speed() {
    level = (score / 5) + 1;
    if (level > 10) level = 10;
    // Adjust Timer1 interval: 0.5s at L1, 0.05s at L10
    uint32_t cmp = 500000 - (level - 1) * 45000;
    TIMER1->CMP = cmp;
}

// Spawn new shape
void Spawn_New_Shape() {
    currentShape = shapes[nextShapeIndex];
    currentX = GRID_WIDTH / 2 - 2;
    currentY = -2; // Start above grid
    currentRotation = 0;
    if (Check_Collision(currentX, currentY, &currentShape)) {
        SetState(STATE_GAME_OVER); // Game over if collision on spawn
    }
}

// gameLogic.c
void Rotate_Current_Block() {
    Tetromino rotated = currentShape;
    Rotate_Clockwise(&rotated);
    if (!Check_Collision(currentX, currentY, &rotated)) {
        currentShape = rotated;
    }
}

void Move_Block_Left() {
    if (!Check_Collision(currentX - 1, currentY, &currentShape)) {
        currentX--;
    }
}

void Move_Block_Right() {
    if (!Check_Collision(currentX + 1, currentY, &currentShape)) {
        currentX++;
    }
}

void Drop_Block_Fast() {
    while (!Check_Collision(currentX, currentY + 1, &currentShape)) {
        currentY++;
    }
    Merge_Shape_Into_Grid();
    int lines = Clear_Lines();
    if (lines > 0) {
        score += lines;
        Update_Level_Speed();
    }
    Spawn_New_Shape();
}


