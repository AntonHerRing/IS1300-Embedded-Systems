/**
******************************************************************************
@brief 		Crossing, program simulating a 4-way crossing
@file 		Crossing.c
@author 		Anton Herdin Ringstedt
@version 1.0
@date 		13-December-2022
@brief 		The program containing Crossing state-machine
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Crossing.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"
#include "Crossing_functions.h"
#include "Trafficlight_functions.h"

/**
 @variables	struct containing state machine states
 */
typedef enum
{ Base,
Lane1,
Lane2
}states;

static states State, NextState;

/**
@brief 		Crossing, state machine for program itself
@param1 	void, no parameter
@return 	void, no return value
*/
void Crossing(void)
{
	uint8_t bytes = 3;
	uint32_t buffer[1];

	uint16_t OrangeDelay = 2000;
	uint16_t GreenDelay = 5000;
	uint16_t RedDelayMax = 10000;

	uint32_t time = 0;
	uint32_t Orngtime = 0;


	State = Base;
	while(1)
	{
		switch(State)
		{
			case Base:
				time = HAL_GetTick();
				while(1) //stay while no cars
				{
					if(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET)
					{
						NextState = Lane2;
						break;
					}
					if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET)
					{
						NextState = Lane1;
						break;
					}

					//toggle between lanes
					if((HAL_GetTick() - time) == (GreenDelay - OrangeDelay) || (HAL_GetTick() - time) == (GreenDelay * 2 - OrangeDelay))
						buffer[0] = setTraffic(2,2); 	//both lanes orange
					if((HAL_GetTick() - time) == GreenDelay && !readLane1(4, buffer[0]))
						buffer[0] = setTraffic(4,1); 	//lane1 green, lane2 red
					else if((HAL_GetTick() - time) == GreenDelay*2 && !readLane2(4, buffer[0]))
					{
						buffer[0] = setTraffic(1,4); 	//lane1 red, lane2 green
						time = HAL_GetTick();
					}
					ShiftLED(buffer, bytes);
				}
				Orngtime = HAL_GetTick();

			break;

			case Lane1:
				time = HAL_GetTick();
				NextState = Base;
				while((HAL_GetTick() - time) <= RedDelayMax)	//stay while lane2 no cars
				{
					if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_RESET)
						break;

					//transition orange
					if((HAL_GetTick() - Orngtime) <= OrangeDelay && !readLane1(4, buffer[0]))
						buffer[0] = setTraffic(2,2);
					else
						buffer[0] = setTraffic(4,1); 	//lane1 green, lane2 red
					ShiftLED(buffer, bytes);
				}
				//change lane
				if(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET)
				{
					NextState = Lane2;
					Orngtime = HAL_GetTick();
				}

			break;

			case Lane2:
				time = HAL_GetTick();
				NextState = Base;
				while((HAL_GetTick() - time) <= RedDelayMax)	//stay while lane1 no cars
				{
					if(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_RESET)
						break;

					//transition orange
					if((HAL_GetTick() - Orngtime) <= OrangeDelay && !readLane2(4, buffer[0]))
						buffer[0] = setTraffic(2,2);
					else
						buffer[0] = setTraffic(1,4); 	//lane1 red, lane2 green
					ShiftLED(buffer, bytes);
				}
				//change lane
				if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET)
				{
					NextState = Lane1;
					Orngtime = HAL_GetTick();
				}
			break;

		}
		State = NextState;
	}
}
