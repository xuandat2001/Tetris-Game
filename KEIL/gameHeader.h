/* Define Standard Colors (RGB565 format) */
#define C_WHITE     0xFFFF
#define C_BLACK     0x0000
#define C_BLUE      0x001F
#define C_BLUE2     0x051F
#define C_RED       0xF800
#define C_MAGENTA   0xF81F
#define C_GREEN     0x07E0
#define C_CYAN      0x7FFF
#define C_YELLOW    0xFFE0
#define SQUARE_SIZE 10


#define COLOR_CYAN    0x07FF  // I shape
#define COLOR_BLUE    0x001F  // J shape
#define COLOR_ORANGE  0xFD20  // L shape
#define COLOR_YELLOW  0xFFE0  // O shape
#define COLOR_GREEN   0x07E0  // S shape
#define COLOR_PURPLE  0x780F  // T shape
#define COLOR_RED     0xF800  // Z shape

#define COLOR_BORDER  0x0000  // Black border
// Define colors (RGB565)
#define COLOR_WHITE   0xFFFF
#define COLOR_ORANGE  0xFD20
#define COLOR_BLUE    0x001F
// Characters
extern uint8_t Font8x16[];
extern uint16_t Font16x32[];
extern const uint16_t gImage_Dog[240*320];
extern const uint16_t WelcomeScreen [240*320];
extern const uint16_t backgroundScreenPlay [240*320];

void LCD_Picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *img);

void LCD_Draw_FilledSquare(uint16_t x, uint16_t y, uint16_t fillColor, uint16_t borderColor);

void LCD_Draw_SquareBorder(uint16_t x, uint16_t y, uint16_t rows, uint16_t cols, uint16_t fillColor, uint16_t borderColor);
void Draw_Z_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void Draw_T_Shape(uint16_t x, uint16_t y,  uint16_t borderColor);
void Draw_O_Shape(uint16_t x, uint16_t y,  uint16_t borderColor);
void Draw_S_Shape(uint16_t x, uint16_t y,  uint16_t borderColor);
void Draw_L_Shape(uint16_t x, uint16_t y,  uint16_t borderColor); 
void Draw_J_Shape(uint16_t x, uint16_t y,  uint16_t borderColor);
void Draw_I_Shape(uint16_t x, uint16_t y, uint16_t borderColor);
void LCD_Draw_Border();
void gameLoop();