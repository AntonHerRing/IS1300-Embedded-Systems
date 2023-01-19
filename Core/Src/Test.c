/*
 * Test.c
 *
 *  Created on: Nov 21, 2022
 *      Author: anton
 */
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Trafficlight_functions.h"
#include "Crossing_functions.h"
#include "gpio.h"
#include "spi.h"

void Test_program(void)
{
	//Test_ShiftLeds();
	//switchTLtest();
	//buttontest();
	//traffictest();
	//carlighttest();
	//alllightstest();
	//timeModulutest();
}

void Test_ShiftLeds(void)
{
	uint8_t bytes = 3;
	uint32_t leds1[] = { 0x000007 }; 		//traffic light 1 (led 7 - 9)
	uint32_t leds2[] = { 0x000007 << 3 }; 	//traffic light 2 (led 10 - 12)
	uint32_t leds3[] = { 0x000007 << 8 }; 	//traffic light 3 (led 4 - 6)
	uint32_t leds4[] = { 0x000007 << 11 }; 	//pedestrian(parallel) light 1 (led 19 - 22) + indicator (led 23 - 24)
	uint32_t leds5[] = { 0x000007 << 16 }; 	//traffic light 4 (led 1 - 3)
	uint32_t leds6[] = { 0x000007 << 19 }; 	//pedestrian(parallel) light 2 (led 13 - 16) + indicator (led 17 - 18)

	ShiftLED(leds1, bytes);
	HAL_Delay(1000);

	ShiftLED(leds2, bytes);
	HAL_Delay(1000);

	ShiftLED(leds3, bytes);
	HAL_Delay(1000);

	ShiftLED(leds4, bytes);
	HAL_Delay(1000);

	ShiftLED(leds5, bytes);
	HAL_Delay(1000);

	ShiftLED(leds6, bytes);
	HAL_Delay(1000);

}

void switchTLtest(void)
{
	uint8_t bytes = 3;
	uint32_t leds[] = { 0x000000 }; 		//reset
	uint32_t leds1[] = { 0x000007 }; 		//traffic light 1 (led 7 - 9)
	uint32_t leds2[] = { 0x000007 << 3 }; 	//traffic light 2 (led 10 - 12)
	uint32_t leds3[] = { 0x000007 << 8 }; 	//traffic light 3 (led 4 - 6)
	//uint32_t leds4[] = { 0x000007 << 11 }; 	//pedestrian(parallel) light 1 (led 19 - 22) + indicator (led 23 - 24)
	uint32_t leds5[] = { 0x000007 << 16 }; 	//traffic light 4 (led 1 - 3)
	//uint32_t leds6[] = { 0x000007 << 19 }; 	//pedestrian(parallel) light 2 (led 13 - 16) + indicator (led 17 - 18)

	while(1)
	{
		if(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET)
			ShiftLED(leds5, bytes);

		if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET)
			ShiftLED(leds3, bytes);

		if(HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET)
			ShiftLED(leds1, bytes);

		if(HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET)
			ShiftLED(leds2, bytes);

		HAL_Delay(1);
		ShiftLED(leds, bytes);
	}

}

void buttontest(void)
{
	uint8_t bytes = 3;
	uint32_t leds[] = { 0x000000 }; 		//reset
	uint32_t leds4[] = { 0x000007 << 11 }; 	//pedestrian(parallel) light 1 (led 19 - 22) + indicator (led 23 - 24)
	uint32_t leds6[] = { 0x000007 << 19 }; 	//pedestrian(parallel) light 2 (led 13 - 16) + indicator (led 17 - 18)

	while(1)
	{
		if(HAL_GPIO_ReadPin(GPIOA ,PL1_Switch_Pin) == GPIO_PIN_RESET)
			ShiftLED(leds6, bytes);

		if(HAL_GPIO_ReadPin(GPIOB ,PL2_Switch_Pin) == GPIO_PIN_RESET)
			ShiftLED(leds4, bytes);

		HAL_Delay(1);
		ShiftLED(leds, bytes);
	}

}

void traffictest(void)
{
	int n[] = {0, 1, 2, 4, 0};
	uint8_t lane1, lane2;
	uint32_t shiftbits[] = {0};
	uint8_t bytes = 3;

	//lane1
	for(int i = 0; i < 5; i++)
	{
		lane1 = 0;
		lane2 = n[i];
		shiftbits[0] = setTraffic(lane1, lane2);
		ShiftLED(shiftbits, bytes);
		HAL_Delay(1000);
	}

	//lane2
	for(int i = 0; i < 5; i++)
	{
		lane1 = n[i];
		lane2 = 0;
		shiftbits[0] = setTraffic(lane1, lane2);
		ShiftLED(shiftbits, bytes);
		HAL_Delay(1000);
	}

	//both lanes
	for(int i = 0; i < 5; i++)
	{
		lane1 = n[i];
		lane2 = n[4 - i];
		shiftbits[0] = setTraffic(lane1, lane2);
		ShiftLED(shiftbits, bytes);
		HAL_Delay(1000);
	}

}

void carlighttest(void)
{
	uint8_t lane1, lane2;
	uint32_t shiftbits[] = {0};
	uint8_t bytes = 3;
	uint8_t i = 0;
	uint8_t j = 0;
	int n[] = {1, 2, 4};

	while(1)
	{
		if((HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET) &&
			(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_RESET))
		{
			if(i < 3) //lane2 on
			{
				lane1 = n[2-i]; //ends in red
				lane2 = n[i];
				shiftbits[0] = setTraffic(lane1, lane2);
				i++;
			}
		}
		else if((HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET) &&
				(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_RESET))
		{
			if(j < 3) //lane1 on
			{
				lane1 = n[j];	//ends in green
				lane2 = n[2-j];
				shiftbits[0] = setTraffic(lane1, lane2);
				j++;
			}
		}
		else
		{
			shiftbits[0] = 0;
			i = 0;
			j = 0;
		}

		ShiftLED(shiftbits, bytes);
		HAL_Delay(1000);
	}

}

void alllightstest(void)
{
	uint8_t bytes = 3;
	uint32_t buffer[] = {0};
	while(1)
	{
		buffer[0] |= setTraffic(4,4) | setPedestrian(2, 2);

		ShiftLED(buffer, bytes);
		HAL_Delay(1000);
	}
}

void timeModulutest(void)
{
	uint32_t leds1[] = { 0x000007 };
	uint32_t reset[] = { 0x000000 };
	uint16_t GreenDelay = 1000;

	uint8_t toggle = 0;
	while(1)
	{
		if(HAL_GetTick() % GreenDelay == 0)
			if(toggle = 0)
			{
				ShiftLED(leds1, 3);
				toggle = 1;
			}
			else
			{
				ShiftLED(reset, 3);
				toggle = 0;
			}
	}
}



