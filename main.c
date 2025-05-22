/******************************************************************************
 * @file     main.c
 * @brief    This is a M480 EBI sample code to drive the LCD displaying and
 *           also utilize ADC to judge the position of finger touch.
 * @version  1.0.0
 * @Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "NuMicro.h"
#include "EBI_LCD_Module.h"
#include "KEIL/gameHeader.h"
#include "KEIL/gameState.h"
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

		SYS_LockReg(); // Lock the register
}

void TMR1_IRQHandler(void)
{
		if (TIMER1->INTSTS & (1 << 0))
		{
			TIMER1->INTSTS = (1 << 0); // Clear Timer 1 overflow flag	
			
			/* Set Timer1_flag = 1 */
			Timer1_flag = 1;

			/* Timer1_cnt + 1 */
			Timer1_cnt = Timer1_cnt + 1;
		}
}

void Timer1_Init(void)
{
		// Set Prescale
		TIMER1->CTL &= ~(0xFF << 0); // clear current setting for Prescale
		TIMER1->CTL |= (0 << 0); // Prescale = (0+1) = 1
	
    /*( 1/12MHz * 1200000) - 1 = 100ms */
    TIMER1->CMP = 1199999;
	
		// Set TM1 operation mode to Periodic Mode
		TIMER1->CTL &= ~(0b11 << 27); // Clear current settings
		TIMER1->CTL |= (0b01 << 27);	// Periodic Mode
		// The behavior selection in periodic mode is Enabled.
		TIMER1->CTL |= (1 << 20);
		// Enable TM1 interrup flag TIF
		TIMER1->CTL |= (1 << 29);

		// Configure Interrupt
		// Enable TM1 interrup flag TIF
		TIMER1->CTL |= (1 << 29);
		// NVIC interrupt configuration
		NVIC->ISER[1] |= (1 << 1); // (33 - 32 = 1)
		// Clear Timer 1 overflow flag
		TIMER1->INTSTS = (1 << 0); // Write 1 to clear TIF
		
		// TM1 Start Counting
		TIMER1->CTL |= (1 << 30);

    /* Clear Timer1_flag */
    Timer1_flag = 0;

    /* Reset Timer1_cnt */
    Timer1_cnt = 0;
}

void UART0_Config(void)
{
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


void GPA_IRQHandler(void)
{
	if (PA->INTSRC & (1 << 0)) // Check if the interrupt is from PA.0 (SW1)
    {
		PA->INTSRC |= (1 << 0); // Clear the interrupt flag

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

void Joystick_Polling_Read(void)
{
    if (!(PG->PIN & (1 << 2))) // UP
        printf("Joystick UP pressed\n");
				//PH->DOUT |= (1 << 6); // toggle LED2

    if (!(PG->PIN & (1 << 4))) // RIGHT
        printf("Joystick RIGHT pressed\n");
				//PH->DOUT ^= (1 << 7); // toggle LED1

    if (!(PC->PIN & (1 << 10))) // DOWN
        printf("Joystick DOWN pressed\n");

    if (!(PC->PIN & (1 << 9))) // LEFT
        printf("Joystick LEFT pressed\n");
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
    SYS_Init();
		Joystick_Init();
	
	  /* Init UART to 115200-8n1 for print message */
    //UART_Open(UART0, 115200);
		UART0_Config();
	
	  printf("\n");
    printf("+---------------------------------------------------------+\n");
    printf("|            M487 LCD Display with Touch Function         |\n");
    printf("+---------------------------------------------------------+\n");
    printf("HXT clock %d Hz\n", CLK_GetHXTFreq());
    printf("CPU clock %d Hz\n", CLK_GetCPUFreq());
	
		/* Initialize EBI bank0 to access external LCD Module */
    //EBI_Open(EBI_BANK0, EBI_BUSWIDTH_16BIT, EBI_TIMING_NORMAL, 0, EBI_CS_ACTIVE_LOW);
    //EBI->CTL0 |= EBI_CTL0_CACCESS_Msk;
    //EBI->TCTL0 |= (EBI_TCTL0_WAHDOFF_Msk | EBI_TCTL0_RAHDOFF_Msk);
		EBI_Config();
    printf("\n[EBI CTL0:0x%08X, TCLT0:0x%08X]\n\n", EBI->CTL0, EBI->TCTL0);

    /* Init LCD Module */
    ILI9341_Initial();

    /* PB.7 BL_CTRL pin */
    // PB7 = 1;
		PB->DOUT |= (1 << 7);

    /* Set PH7/PH6 as output mode for LED1/LED2 */
    //GPIO_SetMode(PH, BIT7|BIT6, GPIO_MODE_OUTPUT);
    //PH7 = 1;
    //PH6 = 1;
		GPIO_Config();

    /* Init ADC for TP */
    /* Set input mode as single-end and enable the A/D converter */
    // EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);
		//EADC->CTL &= ~(1 << 0); // set the Single-end input mode
		//EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);
		EADC_Config();
    /* Init Timer3 */
    //Timer3_Init();
		
		    /* Init Timer1 */
    Timer1_Init();
    /*=== Show the 1st display ===*/
    /* Blank screen */
    LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);

    /* Show text */
    LCD_PutString(0, 144, (uint8_t *)"Welcome to Embedded System",  C_YELLOW, C_BLACK);
		LCD_PutString(0, 160, (uint8_t *)"Design and Implementation",  C_YELLOW, C_BLACK);
		
    /* waiting 3s */
    Timer1_cnt = 0;
    while(Timer1_cnt < 30) {};

    /*=== Show the 2nd display ===*/
    /* Blank screen */
    LCD_BlankArea(0, 0, LCD_W, LCD_H, C_BLACK);
		LCD_PutString(0, 144, (uint8_t *)"Please touch the screen of LCD",  C_YELLOW, C_BLACK);
    while(1) {
				
        if(Timer1_flag == 1) {
            Timer1_flag = 0;
					
						Joystick_Polling_Read();	
						
					  /* Touch scanning */
            x = Get_TP_X();
            y = Get_TP_Y();
					
						if ((x != LCD_W - 1) && (y != LCD_H - 1)) {
								if (touch_count == 0 && !rect_drawn) {
										LCD_Picture(0, 0, 240, 320, WelcomeScreen);
										rect_drawn = 1;
										touch_count++;
								}
								else if (touch_count == 1){
									gameLoop();
								}
						}					
    }
  }

}


