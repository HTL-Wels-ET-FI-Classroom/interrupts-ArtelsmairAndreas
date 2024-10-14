/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * Description of project
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"
#include "ts_calibration.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static volatile int timer_switch = 0;

static volatile int color_select = 0;

static volatile int cnt1 = 0;

static volatile int cnt2 = 0;

/* Private function prototypes -----------------------------------------------*/
static int GetUserButtonPressed(void);
static int GetTouchState (int *xCoord, int *yCoord);

/**
 * @brief This function handles System tick timer.
 */

void SysTick_Handler(void){
	HAL_IncTick();
	if(timer_switch == 0){
		cnt1 ++;
	} else if(timer_switch == 1){
		cnt2 ++;
	}
}

//ISR
void EXTI0_IRQHandler(void){
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
	if (timer_switch == 0) {
		timer_switch = 1;
	} else if (timer_switch == 1){
		timer_switch = 0;
	}
}

//ISR
void EXTI4_IRQHandler(void){
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);

	color_select=! color_select;
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize LCD and touch screen */
	LCD_Init();
	TS_Init(LCD_GetXSize(), LCD_GetYSize());
	/* touch screen calibration */
	//	TS_Calibration();

	/* Clear the LCD and display basic starter text */
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_YELLOW);
	LCD_SetBackColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font20);
	// There are 2 ways to print text to screen: using printf or LCD_* functions
	LCD_DisplayStringAtLine(0, "    HTL Wels");
	// printf Alternative
	LCD_SetPrintPosition(1, 0);
	printf(" Fischergasse 30");
	LCD_SetPrintPosition(2, 0);
	printf("   A-4600 Wels");

	LCD_SetFont(&Font8);
	LCD_SetColors(LCD_COLOR_MAGENTA, LCD_COLOR_BLACK); // TextColor, BackColor
	LCD_DisplayStringAtLineMode(39, "copyright Andreas_Artelsmair", CENTER_MODE);

	//GPIO (S. 265)
	GPIO_InitTypeDef user;

	user.Alternate = 0;
	user.Pin = GPIO_PIN_0;
	user.Mode = GPIO_MODE_IT_RISING;
	user.Speed = GPIO_SPEED_FAST;
	user.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &user);

	//NVIC (S. 368)
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	//Weiterer Interrupt wird PB4 verwendet
	GPIO_InitTypeDef color_switch;

	color_switch.Alternate = 0;
	color_switch.Pin = GPIO_PIN_4;
	color_switch.Mode = GPIO_MODE_IT_RISING;
	color_switch.Speed = GPIO_SPEED_FAST;
	color_switch.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &color_switch);

	//NVIC (S. 368)
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	/* Infinite loop */

	int farbe = LCD_COLOR_BLUE;

	while (1)
	{
		//execute main loop every 100ms

		if (color_select == 0) {
			farbe = LCD_COLOR_BLUE;
		} else if (color_select == 1) {
			farbe = LCD_COLOR_GRAY;
		}

		// display timer
		LCD_SetFont(&Font20);
		LCD_SetTextColor(farbe);
		LCD_SetPrintPosition(5, 0);
		printf("   Timer1: %.1f", cnt1/1000.0);

		LCD_SetPrintPosition(7, 0);
		printf("   Timer2: %.1f", cnt2/1000.0);

	}
}

/**
 * Check if User Button has been pressed
 * @param none
 * @return 1 if user button input (PA0) is high
 */
static int GetUserButtonPressed(void) {
	return (GPIOA->IDR & 0x0001);
}

/**
 * Check if touch interface has been used
 * @param xCoord x coordinate of touch event in pixels
 * @param yCoord y coordinate of touch event in pixels
 * @return 1 if touch event has been detected
 */
static int GetTouchState (int* xCoord, int* yCoord) {
	void    BSP_TS_GetState(TS_StateTypeDef *TsState);
	TS_StateTypeDef TsState;
	int touchclick = 0;

	TS_GetState(&TsState);
	if (TsState.TouchDetected) {
		*xCoord = TsState.X;
		*yCoord = TsState.Y;
		touchclick = 1;
		if (TS_IsCalibrationDone()) {
			*xCoord = TS_Calibration_GetX(*xCoord);
			*yCoord = TS_Calibration_GetY(*yCoord);
		}
	}

	return touchclick;
}
