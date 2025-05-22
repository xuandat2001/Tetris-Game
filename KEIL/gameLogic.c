#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "NuMicro.h"
#include <math.h>
#include "gameLogic.h"
#include "gameHeader.h"
#include "../EBI_LCD_Module.h"
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