/*
 *  ts_calibrate.c
 *
 *  Copyright (C) 2022 Nuvoton Technology
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NuMicro.h"
#include "GUI.h"
#include "M48XTouchPanel.h"

#ifdef __USE_SD__
#include "diskio.h"
#include "ff.h"

extern FIL hFile;
#endif

#define POINT_NUM           5   //7
#define MAX_NUM             200

#define DIFF                    5

typedef struct
{
    int x;
    int y;
} coordiante;

// physical.x, y is the coordinate of H/W for LCD.
// logical x,y is the coordinate of GUI.
coordiante physical[POINT_NUM], logical[POINT_NUM];

// samples x, y for calibration. maximum number is adjustable MAX_NUM
// the duration is at least 2 seconds for clicking down, and then release

int sampleX[MAX_NUM], sampleY[MAX_NUM];
/*
// Binary linear equations are as follows, it is the key of solution.
// logical.x = physical.x X A + physical.y X B + C
// logical.y = physical.x X D + physical.y X E + F
// Xl = A x Xp + B x Yp + C
// Yl = D x Xp + E x Yp + F
// We use Xp and Yp to be the multiple of the above equations, and obtain the following equations.
// Xl = A x Xp + B x Yp + C
// Xl x Xp = A x Xp x Xp + B x Yp x Xp + C x Xp
// Xl x Yp = A x Xp x Yp + B x Yp x Yp + C x Yp
// we obtain A, B, C from the above 3 equations, by accumulating the coordinates of 5 points.
// Yl = D x Xp + E x Yp + F
// Yl x Xp = D x Xp x Xp + E x Yp x Xp + F x Xp
// Yl x Yp = D x Xp x Yp + E x Yp x Yp + F x Yp
// we obtain D, E, F from the above 3 equations, by accumulating the coordinates of 5 points.
// the value of calibration : A B C D E F (65536), 7 integers used, 1 integer reserved
*/
int cal_values[8];


int ts_writefile(void);
int ts_readfile(void);
int ts_calibrate(int xsize, int ysize);
int ts_phy2log(int *logx, int *logy);

int DisplayFailStatus(int xsize, int ysize)
{
    int i;

    GUI_SetFont(&GUI_Font32_ASCII);
    GUI_SetTextMode(GUI_TM_XOR);

    for (i=0; i<3; i++)
    {
        GUI_DispStringHCenterAt("Fail to calibrate", xsize / 2, ysize / 4 + 30);
        GUI_X_Delay(500);
        GUI_DispStringHCenterAt("Fail to calibrate", xsize / 2, ysize / 4 + 30);
        GUI_X_Delay(500);
    }

    return -1;
}

void write_cross(int x, int y)
{
    GUI_DrawLine(x - 12, y, x - 2, y);
    GUI_DrawLine(x + 2, y, x + 12, y);
    GUI_DrawLine(x, y - 12, x, y - 2);
    GUI_DrawLine(x, y + 2, x, y + 12);
    GUI_DrawLine(x - 6, y - 12, x - 12, y - 12);
    GUI_DrawLine(x - 12, y - 12, x - 12, y - 6);
    GUI_DrawLine(x - 12, y + 6, x - 12, y + 12);
    GUI_DrawLine(x - 12, y + 12, x - 6, y + 12);
    GUI_DrawLine(x + 6, y + 12, x + 12, y + 12);
    GUI_DrawLine(x + 12, y + 12, x + 12, y + 6);
    GUI_DrawLine(x + 12, y - 6, x + 12, y - 12);
    GUI_DrawLine(x + 12, y - 12, x + 6, y - 12);
}

static int compare_x(const void* arg1, const void *arg2)
{
    int ret;
    ret = *(int *)arg1 -*(int *)arg2;
    if ( ret > 0 )
        return 1;
    else if ( ret < 0 )
        return -1;
    return 0;
}

static int compare_y(const void* arg1, const void *arg2)
{
    int ret;
    ret = *(int *)arg1 -*(int *)arg2;
    if ( ret > 0 )
        return 1;
    else if ( ret < 0 )
        return -1;
    return 0;
}

int read_phy_xy(int *x, int *y)
{

    int number, middle;
    int phyx, phyy;

    printf("read physical x, y\n");
again:
    while (1)
    {
        if ( Read_TouchPanel(&phyx, &phyy) > 0 )
        {
            if ( (phyx < 0) || ( phyy < 0 ) )
                continue;
            break;
        }
    }

// Delay 50 ms to wait HW ADC to be ready
    GUI_X_Delay(50);
    number = 0;
    while (1)
    {
        if ( Read_TouchPanel(&phyx, &phyy) > 0)
        {
            sampleX[number] = phyx;
            sampleY[number] = phyy;
        }
        else
        {
            break;
        }
        if ( number < MAX_NUM-1)
            number++;
    }

    printf("Capture %d samples\n",number);

// pick the average value of the middle for the coordinate x, y
    middle = number/2;
    qsort(sampleX, number, sizeof(int), compare_x);
    *x = (sampleX[middle-1] + sampleX[middle]) / 2;

    qsort(sampleY, number, sizeof(int), compare_y);
    *y = (sampleY[middle-1] + sampleY[middle]) / 2;

    if ( number <= 10)
        goto again;

// >= DIFF, it means touch screen is not stable. stop the calibration
    if ( abs(sampleY[middle-1] - sampleY[middle]) >= DIFF )
        return 0;
    if ( abs(sampleX[middle-1] - sampleX[middle]) >= DIFF )
        return 0;
    return 1;
}

int run_calibration(void)
{
    int i, no;
    int sum_px, sum_py, sum_pxy, sum_square_px, sum_square_py, sum_lx, sum_lpx, sum_lpy, sum_ly;
    float deter, im11, im12, im13, im21, im22, im23, im31, im32, im33;

// logical.x = physical.x X A + physical.y X B + C
// logical.y = physical.x X D + physical.y X E + F

// multiple physic.y and physic.y as follows
//  logical.x X physical.x = physical.x X A X physical.x+ physical.y X B X physical.x + C X physical.x
//  logical.x X physical.y = physical.x X A X physical.y+ physical.y X B X physical.y + C X physical.y
// we can obtain the paramters A, B, C from 3 x 3 matrix
// In the similiar method, we could also obtain the parameters D, E, F
//  logical.y X physical.x = physical.x X D X physical.x+ physical.y X E X physical.x + F X physical.x
//  logical.y X physical.y = physical.x X D X physical.y+ physical.y X E X physical.y + F X physical.y

    cal_values[6] = 65536;
    sum_px = sum_py = sum_square_px = sum_square_py = sum_pxy = 0;

// Get sums of physical x, y for matrix

    no = POINT_NUM;
    for(i=0; i<POINT_NUM; i++)
    {
        sum_px += physical[i].x;
        sum_py += physical[i].y;
        sum_square_px += (physical[i].x * physical[i].x);
        sum_square_py += (physical[i].y * physical[i].y);
        sum_pxy += (physical[i].x * physical[i].y);
    }
// From 3x3 matix Z, ZX= Y, X, Y is a 3 x 1 matrix
// deter is the determinant for 3 x 3 matix

    im11 = (float)sum_pxy*sum_py - (float)sum_px*sum_square_py;
    im21 = (float)sum_px*sum_pxy - (float)sum_py*sum_square_px;
    im31=  (float)sum_square_px*sum_square_py - (float)sum_pxy*sum_pxy;
    deter = im11*sum_px+im21*sum_py+im31*no;

    if(deter < 0.01f && deter > -0.01f)
    {
        printf("ts_calibrate: No inverse matrix \n");
        return 0;
    }

// Get elements of inverse matrix as follows
    // im11 im12 im13
    // im21 im22 im23
    // im31 im32 im33
    im32 = im11 = im11/deter;
    im12 = (float)(no*sum_square_py - sum_py*sum_py)/deter;
    im22 = im13 = (float)(sum_px*sum_py - no*sum_pxy)/deter;
    im33 = im21 = im21/deter;
    im23 = (float)(no*sum_square_px - sum_px*sum_px)/deter;
    im31= im31/deter;

// Get sums of logical and physical for x calibration
    sum_lx = sum_lpx = sum_lpy = 0;
    for(i=0; i<POINT_NUM; i++)
    {
        sum_lx += logical[i].x;
        sum_lpx += (logical[i].x*physical[i].x);
        sum_lpy += (logical[i].x*physical[i].y);
    }

// get the calibration for A, B, C mapping cal_values[0], cal_values[1], cal_values[2]
    cal_values[0] = (int)((im11*sum_lx + im12*sum_lpx + im13*sum_lpy)* cal_values[6]);
    cal_values[1] = (int)((im21*sum_lx + im22*sum_lpx + im23*sum_lpy)* cal_values[6]);
    cal_values[2] = (int)((im31*sum_lx + im32*sum_lpx + im33*sum_lpy)* cal_values[6]);

// Get sums of logical and physical for y calibration
    sum_ly = sum_lpx = sum_lpy = 0;
    for(i=0; i<POINT_NUM; i++)
    {
        sum_ly += logical[i].y;
        sum_lpx += (logical[i].y*physical[i].x);
        sum_lpy += (logical[i].y*physical[i].y);
    }

// get the calibration for D, E, F mapping cal_values[3], cal_values[4], cal_values[5]
    cal_values[3] = (int)((im11*sum_ly + im12*sum_lpx + im13*sum_lpy)* cal_values[6]);
    cal_values[4] = (int)((im21*sum_ly + im22*sum_lpx + im23*sum_lpy)* cal_values[6]);
    cal_values[5] = (int)((im31*sum_ly + im32*sum_lpx + im33*sum_lpy)* cal_values[6]);

    return 1;
}


int read_calib_sample(char *rect_name, int index, int logx, int logy)
{
    int result;
	  GUI_SetColor(GUI_WHITE);
    write_cross(logx, logy);
    result = read_phy_xy (&physical[index].x, &physical[index].y);
	  GUI_SetColor(GUI_BLACK);
    write_cross(logx, logy);

    logical[index].x = logx;
    logical[index].y = logy;

    printf("%s : X = %4d Y = %4d\n", rect_name, physical[index].x, physical[index].y);
    return result;
}

int ts_calibrate(int xsize, int ysize)
{
    int result,d1, d2, d3;
    GUI_SetBkColor(GUI_BLACK);
	  GUI_Clear();

	  if ( xsize >= 800 ) // 800x480
		{
        GUI_SetFont(&GUI_Font24B_ASCII);
			  d1 = -50;
			  d2 = -20;
			  d3 = 10;
		} 
		else if ( xsize >= 480 ) // 480 x272
		{
        GUI_SetFont(&GUI_Font13B_ASCII);
				d1 = -50;
			  d2 = -30;
			  d3 = -10;
		}
		else
		{
			  GUI_SetFont(&GUI_Font8_ASCII);
				d1 = -20;
			  d2 = -10;
			  d3 = 0;
		}

    GUI_SetColor(GUI_RED);
    GUI_DispStringHCenterAt("Nuvoton's TS calibration utility", xsize / 2, ysize / 4 +d1);

    GUI_SetColor(GUI_BLUE);
    GUI_DispStringHCenterAt("Touch crosshair to calibrate", xsize / 2, ysize / 4 +d2 );

    GUI_SetColor(GUI_GREEN);
    GUI_DispStringHCenterAt("Click down 2 seconds and release", xsize / 2, ysize / 4 + d3);

    printf("xsize = %d, ysize = %d\n", xsize, ysize);

    //GUI_SetColor(GUI_WHITE);
    //GUI_SetDrawMode(GUI_DRAWMODE_XOR);
    GUI_SetPenSize(1);

    //Top left
    result = read_calib_sample ("Top left", 0, 40, 40);
    GUI_X_Delay(300);
    if ( result == 0 )
    {
        return(DisplayFailStatus(xsize, ysize));
    }

    //Top right
    result = read_calib_sample ("Top right", 1, xsize - 40, 40);
    GUI_X_Delay(300);
    if ( result == 0 )
    {
        return(DisplayFailStatus(xsize, ysize));
    }

    //Bottom right
    result = read_calib_sample ("Bottom right", 2, xsize - 40, ysize - 40);
    GUI_X_Delay(300);
    if ( result == 0 )
    {
        return(DisplayFailStatus(xsize, ysize));
    }

    //Bottom left
    result = read_calib_sample ("Bottom left", 3, 40, ysize - 40);
    GUI_X_Delay(300);
    if ( result == 0 )
    {
        return(DisplayFailStatus(xsize, ysize));
    }

    // Center
    result = read_calib_sample ("Center", 4, xsize / 2,  ysize / 2);
    GUI_X_Delay(300);
    if ( result == 0 )
    {
        return(DisplayFailStatus(xsize, ysize));
    }

// After running API run_calibration, global variable cal_values is exported.
    if (run_calibration())
    {
        printf("Calibration OK.\n");
        result = 1;
    }
    else
    {
        printf("Calibration failed.\n");
        result = -1;
    }

    return result;
}

// logical.x = physical.x X A + physical.y X B + C
// logical.y = physical.x X D + physical.y X E + F
int ts_phy2log(int *logx, int *logy)
{
    int phyx,phyy;

    phyx = *logx;
    phyy = *logy;
    *logx = ( phyx*cal_values[0] + phyy*cal_values[1] + cal_values[2] ) / cal_values[6];
    *logy = ( phyx*cal_values[3]+  phyy*cal_values[4] + cal_values[5] ) / cal_values[6];
    return 1;
}

// write the calibration parameters into one file
int ts_writefile(void)
{
#ifdef __USE_SD__
    size_t cnt;
    FRESULT res;
    res = f_lseek(&hFile, 0);
    if (res != FR_OK)
    {
        printf("CANNOT seek the calibration into file\n");
        return -1;
    }

    res = f_write(&hFile, (char *)&cal_values[0], 28, &cnt);
    if (res != FR_OK)
    {
        printf("CANNOT write the calibration into file, %d\n", cnt);
        return -1;
    }
#else
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x00, cal_values[0]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x04, cal_values[1]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x08, cal_values[2]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x0C, cal_values[3]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x10, cal_values[4]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x14, cal_values[5]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x18, cal_values[6]);
    FMC_Write(__DEMO_TSFILE_ADDR__ + 0x1C, 0x55AAA55A);
#endif
    return 0;
}

int ts_readfile(void)
{
#ifdef __USE_SD__
    size_t cnt;
    FRESULT res;
    res = f_lseek(&hFile, 0);
    if (res != FR_OK)
    {
        printf("CANNOT seek the calibration into file\n");
        return -1;
    }

    res = f_read(&hFile, (char *)&cal_values[0], 28, &cnt);
    if (res != FR_OK)
    {
        printf("CANNOT read the calibration into file, %d\n", cnt);
        return -1;
    }
#else
    cal_values[0] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x00);
    cal_values[1] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x04);
    cal_values[2] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x08);
    cal_values[3] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x0C);
    cal_values[4] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x10);
    cal_values[5] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x14);
    cal_values[6] = FMC_Read(__DEMO_TSFILE_ADDR__ + 0x18);
#endif
    return 0;
}
