/******************************************************************************
 * @file     main.c
 * @brief    This file is to call the function to execute states of Tetris game by user's purposes  .
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include <string.h>
#include "EBI_LCD_Module.h"
#include "KEIL/gameHeader.h"
#include "KEIL/gameState.h"
#include "KEIL/gameLogic.h"
#define RECT_WTDTH 50
#define RECT_HIGH 100
#define HXTSTB 1 << 0     // HXT Clock Sourse Stable Flag
#define PLLSTB 1<<2				// Internal PLL Clock Source Stable Flag
#define HXTEN  1<<0				// HXT Enable Bit, write 1 to enable 



/*---------------------------------------------------------------------------*/
/* Define                                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Global variables                                                          */
/*---------------------------------------------------------------------------*/
extern  volatile    uint8_t Timer3_flag;
extern  volatile    uint8_t Timer3_cnt;
//volatile    uint8_t Timer1_flag;
//volatile    uint8_t Timer1_cnt;

/*---------------------------------------------------------------------------*/
/* Functions                                                                 */
/*---------------------------------------------------------------------------*/
volatile    uint8_t     Timer1_flag = 0;
volatile    uint8_t     Timer1_cnt = 0;
volatile    uint8_t     Timer0_flag = 0;
volatile    uint8_t     Timer0_cnt = 0;
volatile uint8_t timer_running = 1; // 1 = running, 0 = paused
volatile uint8_t switchPressed = 0; // Flag for debouncing
uint8_t seconds = 0;
uint8_t minutes = 0;
volatile uint32_t millis = 0;
static uint32_t lastLeftTime = 0;
static uint32_t lastRightTime = 0;
#define DEBOUNCE_DELAY_MS 200  // Same as up/down



void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
		CLK->PWRCTL |= HXTEN; // Enable HXT
	
    /* Waiting for 12MHz clock ready */
		while(!(CLK->STATUS & HXTSTB));
	
    /* Switch HCLK clock source to HXT */
		CLK->CLKSEL0 &= ~(0b111 << 0); //clear
		CLK->CLKSEL0 |= (0b000 << 0); //set

    /* Set core clock as PLL_CLOCK from PLL */
	  //Configure PLL for 192 MHz
		CLK->PLLCTL &=(~(0xFFFF << 0));      // Clear PLLCTL[15:0]
		CLK->PLLCTL &= (~(1 << 19)); 				// PLL Source is HXT
		CLK->PLLCTL &= (~(1 << 16));				// PLL is in normal mode
		
		// Configure PLL output frequency
		// FIN = 12 MHZ; FOUT = 192 MHZ
		// Choose:
		// NR = 2 -> INDIV = 1
		// NF = 32 -> FBDIV = 30
		// NO = 2 -> OUTDIV = "01"
		CLK->PLLCTL |= (1 << 9); 				// INDIV
		CLK->PLLCTL |= (30 << 0); 			// FBDIV
		CLK->PLLCTL &= (0b01 << 14);    // OUTDIV
		
		CLK->PLLCTL &= ~(1 << 18);   // PLL clock enable
    
    while (!(CLK->STATUS & PLLSTB)); // Wait for PLL to stabilize

		// Set HCLK to PLLFOUT
		CLK->CLKSEL0 &= (~(0x07 << 0)); // Clear current settings for 
    CLK->CLKSEL0 |= 0x02; 					// Set a new value
	
		// Set HCLK Divider to 0
		CLK->CLKDIV0 &= (~(0x0F<<0)); // Clear current settings for HCLKDIV
		CLK->CLKDIV0 |= (0x00 << 0);			// Set new value
		
    /* Set both PCLK0 and PCLK1 as HCLK/2 */
		CLK->PCLKDIV &= ~((0x07 << 0) | (0x07 << 4));
		CLK->PCLKDIV |= ((0x1 << 0) | (0x1 << 4));

		/* UART 0 clock setting */
		// Set UART0 clock source to HXT
		CLK->CLKSEL1 &= ~(0b11 << 24); 		// Clear bits 25:24 
		CLK->CLKSEL1 |= (0b00 << 24);       // Set to HXT
		CLK->CLKDIV0 &= ~(0xF << 8);		// Clock divider is 1
		CLK->APBCLK0 |= (1 << 16); 			// Enable UART0 clock		
		
		/* EADC clock setting */
		// EADC clock source is PCLK1 96 MHz
		CLK->CLKDIV0 &= ~(0x0FF << 16);		// Clear current settings
		CLK->CLKDIV0 |= (7 << 16); 		// EADC clock divider is (7+1) --> ADC clock is 96/8 = 12 MHz (Maximum is 72)
		CLK->APBCLK0 |= (1 << 28); 			// enable EADC0 clock
		
		// TM1 clock selection 
		CLK->CLKSEL1 &= ~ (0b111 << 12); // clear setting
		CLK->CLKSEL1 |= (0b000 << 12); 	// Clock source from HXT
		CLK->APBCLK0 |= (1 << 3); 		// Clock enable for Timer 1
		
		// TM0 clock selection 
		CLK->CLKSEL1 &= ~ (0b111 << 8); // clear setting
		CLK->CLKSEL1 |= (0b000 << 8); 	// Clock source from HXT
		CLK->APBCLK0 |= (1 << 2); 		// Clock enable for Timer 0
		
		// TM2 clock selection
		CLK->CLKSEL1 &= ~ (0b111 << 16); // clear setting
		CLK->CLKSEL1 |= (0b000 << 16);  // Clock source from HXT
		CLK->APBCLK0 |= (1 << 4);       // Clock enable for Timer 2

		// EBI Controller Clock Enable Bit
		CLK->AHBCLK |= (1 << 3);  

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/

    /*=== EBI (LCD module) mult-function pins ===*/
    /* EBI AD0~5 pins on PG.9~14 */
    SYS->GPG_MFPH &= ~((0xF << 4) | (0xF << 8)  |
                       (0xF << 12) | (0xF << 16)  |
                       (0xF << 20) | (0xF << 24) );
    SYS->GPG_MFPH |= ((0x02 << 4) | (0x02 << 8) |
                      (0x02 << 12) | (0x02 << 16) |
                      (0x02 << 20) | (0x02 << 24));

    /* EBI AD6, AD7 pins on PD.8, PD.9 */
    SYS->GPD_MFPH &= ~((0xF << 0) | (0xF << 4));
    SYS->GPD_MFPH |= ((0x02<<0) | (0x02<<4));

    /* EBI AD8, AD9 pins on PE.14, PE.15 */
    SYS->GPE_MFPH &= ~((0xF << 24) | (0xF << 28));
    SYS->GPE_MFPH |= ((0x02 << 24) | (0x02 << 28));

    /* EBI AD10, AD11 pins on PE.1, PE.0 */
    SYS->GPE_MFPL &= ~((0xF << 4) | (0xF << 0));
    SYS->GPE_MFPL |= ((0x02<<4) | (0x02<<0));

    /* EBI AD12~15 pins on PH.8~11 */
    SYS->GPH_MFPH &= ~((0xF << 0) | (0xF << 4) |
                       (0xF << 8) | (0xF << 12));
    SYS->GPH_MFPH |= ((0x02 << 0) | (0x02 << 4) |
                      (0x02 << 8) | (0x02 << 12));

    /* Configure PH.3 as Output mode for LCD_RS (use GPIO PH.3 to control LCD_RS) */
		PH->MODE &= ~(0x3 << 6);   // Clear bits 7:6
		PH->MODE |=  (0x1 << 6);   // Set bits 7:6 to 01 for push-pull ouput
		PH->DOUT |= (1 << 3); // Set PH.3 high
		
    /* EBI RD and WR pins on PE.4 and PE.5 */
    SYS->GPE_MFPL &= ~((0xF << 16) | (0xF << 20));
    SYS->GPE_MFPL |= ((0x02 << 16) | (0x02 << 20));

    /* EBI CS0 pin on PD.14 */
    SYS->GPD_MFPH &= ~(0xF << 24);
    SYS->GPD_MFPH |= (0x02 << 24);

    /* Configure PB.6 and PB.7 as Output mode for LCD_RST and LCD_Backlight */
		// Clear current mode for PB.6 and PB.7 (2 bits per pin)
		PB->MODE &= ~((0x3 << 12) | (0x3 << 14)); // clear the current bit for PB.6 and PB.7
		PB->MODE |=  ((0x1 << 12) | (0x1 << 14)); // set the push-pull output
		PB->DOUT |= (1 << 6);  // Set PB.6 high
		PB->DOUT &= ~(1 << 7); // Set PB.7 low

    /*=== UART mult-function pins ===*/
    /* Set PD.2 and PD.3 multi-function pins for UART0 RXD, TXD */
    SYS->GPD_MFPL &= ~((0xF << 8) | (0xF << 12));
    SYS->GPD_MFPL |= ((0x09 << 8) | (0x09 << 12));

		//SYS_LockReg(); // Lock the register
}

void TMR2_IRQHandler(void)
{
		if (TIMER2->INTSTS & (1 << 0))
		{
			TIMER2->INTSTS = (1 << 0); // Clear Timer 0 overflow flag	
			
			millis++;
		}
}

void Timer2_Init(void)
{
		// Set Prescale
		TIMER2->CTL &= ~(0xFF << 0); // clear current setting for Prescale
		TIMER2->CTL |= (0 << 0); // Prescale = (0+1) = 1
	
    // Duration = 1ms => Target Tiner Count = 11999
    TIMER2->CMP = 11999;
	
		// Set TM2 operation mode to Periodic Mode
		TIMER2->CTL &= ~(0b11 << 27); // Clear current settings
		TIMER2->CTL |= (0b01 << 27);	// Periodic Mode
		// The behavior selection in periodic mode is Enabled.
		TIMER2->CTL |= (1 << 20);
		// Enable TM0 interrup flag TIF
		TIMER2->CTL |= (1 << 29);

		// Configure Interrupt
		// Enable TM2 interrup flag TIF
		TIMER2->CTL |= (1 << 29);
		// NVIC interrupt configuration
		NVIC->ISER[1] |= (1 << 2); // (34 - 32 = 2)
		// Clear Timer 2 overflow flag
		TIMER2->INTSTS = (1 << 0); // Write 1 to clear TIF
		
		// TM2 Start Counting
		TIMER2->CTL |= (1 << 30);

}

void TMR1_IRQHandler(void)
{
    // Clear Timer 0 overflow flag
    TIMER1->INTSTS = (1 << 0);
	 // Only move down when timer triggers
    MoveTetrominoDown();
    needsRedraw = 1;  // Set flag for main loop to handle movement

}

void Timer1_Init(void)
{
				// Set Prescale
		TIMER1->CTL &= ~(0xFF << 0); // clear current setting for Prescale
		TIMER1->CTL |= (0 << 0); // Prescale = (0+1) = 1 
    /*( 1/12MHz * 1200000) - 1 = 100ms */
    TIMER1->CMP = 5999999;
			// Set TM1 operation mode to Periodic Mode
		TIMER1->CTL &= ~(0b11 << 27); // Clear current settings
		TIMER1->CTL |= (0b01 << 27);	// Periodic Mode
		// The behavior selection in periodic mode is Enabled.
		TIMER1->CTL |= (1 << 20);
				// Enable TM1 interrup flag TIF
		TIMER1->CTL |= (1 << 29);

    /* Enable Timer1 IRQ */
    // NVIC interrupt configuration
		NVIC->ISER[1] |= (1 << 1);

		// TM1 Start Counting
		TIMER1->CTL |= (1 << 30);

}

void TMR0_IRQHandler(void)
{
		if (TIMER0->INTSTS & (1 << 0))
		{
			TIMER0->INTSTS = (1 << 0); // Clear Timer 0 overflow flag	
			
			/* Set Timer0_flag = 1 */
			Timer0_flag = 1;

			/* Timer0_cnt + 1 */
			Timer0_cnt = Timer0_cnt + 1;
		}
}

void Timer0_Init(void)
{
		// Set Prescale
		TIMER0->CTL &= ~(0xFF << 0); // clear current setting for Prescale
		TIMER0->CTL |= (11 << 0); // Prescale = (11+1) = 12
	
    // Duration = 1s => Target Tiner Count = 499999
    TIMER0->CMP = 999999;
	
		// Set TM0 operation mode to Periodic Mode
		TIMER0->CTL &= ~(0b11 << 27); // Clear current settings
		TIMER0->CTL |= (0b01 << 27);	// Periodic Mode
		// The behavior selection in periodic mode is Enabled.
		TIMER0->CTL |= (1 << 20);
		// Enable TM0 interrup flag TIF
		TIMER0->CTL |= (1 << 29);

		// Configure Interrupt
		// Enable TM0 interrup flag TIF
		TIMER0->CTL |= (1 << 29);
		// NVIC interrupt configuration
		NVIC->ISER[1] |= (1 << 0); // (32 - 32 = 0)
		// Clear Timer 0 overflow flag
		TIMER0->INTSTS = (1 << 0); // Write 1 to clear TIF
		
		// TM0 Start Counting
		TIMER0->CTL |= (1 << 30);

    /* Clear Timer0_flag */
    Timer0_flag = 0;

    /* Reset Timer0_cnt */
    Timer0_cnt = 0;
}

void UART0_Config(void)
{
	
		// Pin configuration
		// PB.12 (RX) --> Input ; PB.13 (TX) --> output
		PB->MODE &= ~(0b11 << 24); // PB.12 as input
	
		PB->MODE &= ~(0b11 << 26); // clear setting PB.13
		PB->MODE |= (0b01 << 26); // PB.13 as output push-pull mode
		
		// Set GPB Multi-function pins for UART0 RXD and TXD
		SYS->GPB_MFPH &= ~ ((0xF << 16) | (0xF << 20)); 	// Clear current settings PB.12 -> [19:16]; PB.13 [23:20]
		SYS->GPB_MFPH |= ( (6 << 16) | (6 << 20) );			// UART0 - MFP6
	
    /* Init UART to 115200-8n1 for print message */	
		// UART 0 operation configuration
		UART0->LINE |= (0b11 << 0); 			// 8 data bit
		UART0->LINE &= ~(1 << 2); 				// One stop bit
		UART0->LINE &= ~(1 << 3);				// No parity bit
		
		UART0->FIFO |= ((1 << 1) 		// Clear RX FIFO
										| (1 << 2)); 	// Clear TX FIFO				
		UART0->FIFO &= ~(0xF << 16); 	// FIFO Trigger level is 1 byte
		
		// Baud rate config: Mode 1 - 115200 bps - Page 1191 - TRM
		UART0->BAUD &= ~((0b11 << 28) | (0xF << 24) | (0xFFFF << 0)); // clear the current bits
		
		UART0->BAUD |= ((0b10 << 28) // set to mode 1
										| (12 << 24) // set the extra divider EDIVM1 = 12
										| 6); 			 // BRD = 6 
		
		/* Calucation backward the Baud rate to check
		Baud Rate = UART_CLK / [ (EDIVM1 + 1) ? (BRD + 2) ]
          = 12,000,000 / [13 ? 8]
          = 12,000,000 / 104
          = 115,384.6 bps */
}
void EBI_Config(void)
{
	  /* Initialize EBI bank0 to access external LCD Module */
		// Configure EBI_CTL0 for bank 0
		EBI->CTL0 &= ~((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (0b111 << 8)); // clear the current bit
	
		EBI->CTL0 |= ((1 << 0) 	// Enable EBI
									| (1 << 1) // 16-bit data bus
									| (0 << 2) // CS active low
									| (0 << 3) // Address/Data not separated
									| (1 << 4)) // Continuous access
									| (0b000 << 8); // MCLKDIV = 0 (HCLK/1)
		
		EBI->TCTL0 &= ~((1 << 22) | (1 << 23)); // clear the current bit
		EBI->TCTL0 |= (1 << 22) 	// Data Access Hold Time (tAHD) during EBI reading Disabled
								| (1 << 23);  // Data Access Hold Time (tAHD) during EBI writing Disabled
	
}
void GPIO_Config(void)
{
	  /* Set PH7/PH6 as output mode for LED1/LED2 */
		// Configure PH.7 and PH.6 as output (each pin uses 2 bits in PH->MODE)
		PH->MODE &= ~((0x3 << 14) | (0x3 << 12));  // Clear mode bits for PH.7 and PH.6
		PH->MODE |=  ((0x1 << 14) | (0x1 << 12));  // Set to push-pull output mode 

		// Set PH.7 and PH.6 high
		PH->DOUT |= (1 << 7) | (1 << 6);
}

void EADC_Config(void)
{
		EADC->CTL &= ~(1 << 8);  // Single-end analog input mode
		EADC->CTL &= ~(0b11 << 6); // clear the current bit
		EADC->CTL |= (0b11 << 6);  // 12-bit resolution
	
		EADC->CTL |= (1 << 0);						// Enable EADC
		while (!(EADC0->PWRM & (1 << 0)));			// Wait for EADC is ready for conversion
	
		// Configure sample module 0 for EADC0_CH1, software trigger
		EADC->SCTL[0] &= ~(0x1F << 16);		// TRGSEL = 0 -> Disable trigger or software trigger
		EADC->SCTL[0] &= ~(0xF << 0);		// Clear settings for channel selection
		EADC->SCTL[0] |= (1 << 0);			// Select EADC0_CH1
	
		EADC->STATUS2 = (1 << 0);			// Clear any previous interrupt flags for sure	
		EADC->SWTRG |= (1 << 0);               // Trigger conversion
}
void SW1_Interrupt_Setup(void)
{
		//Configure PA.0 (SW1) as input mode
		PA->MODE &= ~(0x3 << 0); 		// Clear bits [1:0] for PA.0
		
	
		PA->INTTYPE &= ~(1 << 0); 		// Edge trigger interrupt for PA.0
		PA->INTEN |= (1 << 0); 		// Falling edge interrupt enable
		PA->INTSRC |= (1 << 0);		// Clear any pending interrupt flag for PA.0

		// NVIC interrupt configuration
		NVIC->ISER[0] |= (1 << 16); 		// Enable NVIC for the GPIO interrupt on Port A 											
}
void GPA_IRQHandler(void) {
    if (PA->INTSRC & (1 << 0)) {
        PA->INTSRC |= (1 << 0); // Clear interrupt flag
        // Simple debounce
			volatile uint32_t i;
        for (i = 0; i < 100000; i++);
        if (!(PA->PIN & (1 << 0))) {
            switchPressed = 1;
        }
    }
}

void Joystick_Init(void)
{
    // Set PG.2(UP), PG.4(RIGHT) as input mode
    PG->MODE &= ~((0x3 << 4) | (0x3 << 8)); // PG.2, PG.4
		PG->PUSEL &= ~((0x3 << 4) |        /* clear PUSEL2  */
									(0x3 << 8)); 
		PG->PUSEL |=  ((0x1 << 4) |        /* set 01 = pull-up */
									(0x1 << 8));

    // Set PC.9(LEFT) and PC.10(DOWN) as input mode
    PC->MODE &= ~((0x3 << 18) | (0x3 << 20)); // PC.9, PC.10
		PC->PUSEL &= ~((0x3 << 18) |
										(0x3 << 20));
		PC->PUSEL |=  ((0x1 << 18) |
										(0x1 << 20));
}

void Joystick_Polling_Read(void) {
    static uint8_t up_debounce = 0;
    static uint32_t left_counter = 0, right_counter = 0, down_counter = 0;
		static uint8_t down_debounce = 0;
    #define INITIAL_DELAY 15  // Adjust these values as needed
    #define REPEAT_DELAY 5
 
    // UP (Rotate) - Edge-triggered with debounce
    if (!(PG->PIN & (1 << 2))) {
        if (!up_debounce) {
            volatile uint32_t i;
            for (i = 0; i < 10000; i++); // Short delay
            if (!(PG->PIN & (1 << 2))) {
                Erase_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
                Rotate_Clockwise(&currentTetromino);
                Draw_Current_Shape_Rotate(currentX, currentY, 0x0000, &currentTetromino);
                needsRedraw = 1;
                up_debounce = 1;
            }
        }
    } else {
        up_debounce = 0;
    }
 
    // RIGHT - Debounced movement
    if (!(PG->PIN & (1 << 4))) {  // Right pressed
        if ((millis - lastRightTime) > DEBOUNCE_DELAY_MS) {
            MoveTetrominoRight();
            lastRightTime = millis;
        }
    }

    // LEFT - Debounced movement
    if (!(PC->PIN & (1 << 9))) {  // Left pressed
        if ((millis - lastLeftTime) > DEBOUNCE_DELAY_MS) {
            MoveTetrominoLeft();
            lastLeftTime = millis;
        }
    }
 
    // DOWN (Hard drop) - Single trigger with debounce
    if (!(PC->PIN & (1 << 10))) {
        if (!down_debounce) {
            volatile uint32_t i;
            for (i = 0; i < 10000; i++);
            if (!(PC->PIN & (1 << 10))) {
                HardDrop();
                down_debounce = 1;
            }
        }
    } else {
        down_debounce = 0;
    }
}

void enableTimer(){
		TIMER0->CTL |= (1 << 30);
		TIMER1->CTL |= (1 << 30);
}
void disableTimer(){
		TIMER0->CTL &= ~(1 << 30);
		TIMER1->CTL &= ~(1 << 30);
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
		char acString[32];
		uint16_t x = 0, y = 0;
    uint16_t rect_x = 0, rect_y = 0;
		uint16_t cir_x = 0, cir_y = 0;
		uint8_t rect_drawn = 0;
		uint8_t cir_drawn = 0;
		uint8_t touch_count = 0;

    /* Init System, IP clock and multi-function I/O
       In the end of SYS_Init() will issue SYS_LockReg()
       to lock protected register. If user want to write
       protected register, please issue SYS_UnlockReg()
       to unlock protected register if necessary */
	    /* Unlock protected registers */
    SYS_UnlockReg();
    SYS_Init();
		Joystick_Init();
	
	  /* Init UART to 115200-8n1 for print message */
		UART0_Config();
	
		EBI_Config();

    /* Init LCD Module */
    ILI9341_Initial();

    /* PB.7 BL_CTRL pin */
    // PB7 = 1;
		PB->DOUT |= (1 << 7);

    /* Set PH7/PH6 as output mode for LED1/LED2 */
		GPIO_Config();

    /* Init ADC for TP */
    /* Set input mode as single-end and enable the A/D converter */
		EADC_Config();
    /* Init Timer3 */
    //Timer3_Init();
		
    Timer1_Init();
		Timer1_SetIntervalByLevel(1);
    /*=== Show the 1st display ===*/
		SW1_Interrupt_Setup();
		
		/* Init Timer1 */
    Timer0_Init();
		Timer2_Init();
		/* Lock protected registers */
    SYS_LockReg();	
		TIMER0->CTL &= ~(1 << 30);
		TIMER1->CTL &= ~(1 << 30);

    while (1) {
        switch (currentState) {
            case WELCOME_SCREEN:
                LCD_Picture(0, 0, 240, 320, WelcomeScreen);
                if (switchPressed) {
                    currentState = READY_SCREEN;
                    switchPressed = 0;
                }
                break;

							case READY_SCREEN:
									// Clear entire screen before drawing
									LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK); // Full black screen
									LCD_PutString(50, 100, (uint8_t *)"Press SW1 to Start", C_WHITE, C_BLACK);
									
									if (switchPressed) {
											// Reset game state
											score = 0;
											level = 1;
											seconds = 0;
											minutes = 0;
											memset(board, 0, sizeof(board)); // Clear the board
											Prepare_Next_Shape();
											Promote_Next_Shape();
											currentX = SPAWN_X;
											currentY = SPAWN_Y;
											currentTetromino = CreateTetromino(currentShapeIndex, currentX, currentY, 
																											GetShapeColor(currentShapeIndex));
											// Force UI re-initialization
											needsRedraw = 1;
											// Start timers
											currentState = PLAYING;
											switchPressed = 0;
									}
									break;

            case PLAYING:
								enableTimer();
                Joystick_Polling_Read();
                gameLoop();
                if (switchPressed) {
                    currentState = PAUSED;
                    switchPressed = 0;
                    // Pause timers
                    TIMER0->CTL &= ~(1 << 30);
                    TIMER1->CTL &= ~(1 << 30);
                    isPaused = 1;
                }
                break;

            case PAUSED:
                //LCD_PutString(50, 100, (uint8_t *)"PAUSED - Press SW1", C_WHITE, C_BLACK);
                if (switchPressed) {
                    currentState = PLAYING;
                    switchPressed = 0;
                    // Resume timers
                    TIMER0->CTL |= (1 << 30);
                    TIMER1->CTL |= (1 << 30);
                    isPaused = 0;
                }
                break;

            case GAME_OVER:
								disableTimer();
                LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
                LCD_PutString(50, 100, (uint8_t *)"GAME OVER - Press SW1", C_WHITE, C_BLACK);
                if (switchPressed) {
                    SaveHighScore(score);
                    currentState = HIGH_SCORE;
                    switchPressed = 0;
                }
                break;

            case HIGH_SCORE:
								// Clear screen fully before showing high scores
								LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
								DisplayHighScores();
								if (switchPressed) {
										// Clear screen again before returning to READY_SCREEN
										LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
										currentState = READY_SCREEN;
										switchPressed = 0;
								}
								break;
        }
    }
}