/**
******************************************************************************
@brief 		Trafficlight, program simulating a traffic light
@file 		Trafficlight.c
@author 		Anton Herdin Ringstedt
@version 1.1	at row 75, OrangeDelay was subtracted from PedestrianDelay
@date 		13-December-2022
@brief 		The program containing Trafficlight state-machine
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Trafficlight_functions.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/**
 @variables	struct containing state machine states
 */
typedef enum
{ Base,
Transition,
Go
}states;

static states State, NextState;

/**
@brief 		Trafficlight, state machine for program itself
@param1 	void, no parameter
@return 	void, no return value
*/
void Trafficlight(void)
{
	uint8_t bytes = 3;
	uint32_t buffer[1];
	uint32_t shiftbytes;

	uint16_t ToggleFreq = 500;
	uint16_t PedestrianDelay = 5000;
	uint16_t WalkingDelay = 7000;
	uint16_t orangeDelay = 2000;

	uint32_t time = 0;
	uint32_t togtime = 0;

	uint8_t lastState = 0;

	State = Base;
	while(1)
	{
		switch(State)
		{
			case Base:
				while(HAL_GPIO_ReadPin(GPIOB ,PL2_Switch_Pin) == GPIO_PIN_SET)	//stuck in base state
				{
					shiftbytes = setLights(4);
					buffer[0] = shiftbytes | ( 0x000001 << 11 ); 	//traffic lights green, pedestrian red
					ShiftLED(buffer, bytes);
				}
				lastState = 0;

				if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET)
				{
					NextState = Transition;
					break;
				}

				time = HAL_GetTick();
				togtime = HAL_GetTick();
				while((HAL_GetTick() - time) <= (PedestrianDelay - OrangeDelay))
				{
					shiftbytes = setLights(4);

					if((HAL_GetTick() - togtime) == ToggleFreq)
					{
						buffer[0] = shiftbytes | ( 0x000001 << 11 ); 	//traffic lights green, pedestrian toggle off
						ShiftLED(buffer, bytes);
					}
					else if((HAL_GetTick() - togtime) == ToggleFreq * 2)
					{
						buffer[0] = shiftbytes | (( 0x000004 << 11 ) | ( 0x000001 << 11 )); 	//traffic lights green, pedestrian toggle on
						ShiftLED(buffer, bytes);
						togtime = HAL_GetTick();
					}
				}
				NextState = Transition;

			break;

			case Transition:
				time = HAL_GetTick();
				while((HAL_GetTick() - time) <= orangeDelay) // orange transition
				{
					shiftbytes = setLights(2);

					if((HAL_GetTick() - togtime) == ToggleFreq || lastState == 1)
					{
						buffer[0] = shiftbytes | ( 0x000001 << 11 ); //traffic lights orange, pedestrian toggle off
						ShiftLED(buffer, bytes);
					}
					else if((HAL_GetTick() - togtime) == ToggleFreq * 2 || (HAL_GetTick() - togtime) > ToggleFreq * 2 )
					{
						buffer[0] = shiftbytes | (( 0x000004 << 11 ) | ( 0x000001 << 11 )); //traffic lights orange, pedestrian toggle on
						ShiftLED(buffer, bytes);
						togtime = HAL_GetTick();
					}
				}
				if(lastState == 0)
					NextState = Go;
				else if( lastState == 1)
					NextState = Base;
			break;

			case Go:
				lastState = 1;

				time = HAL_GetTick();
				while((HAL_GetTick() - time) <= WalkingDelay) // orange transition
				{
					shiftbytes = setLights(1);

					buffer[0] = shiftbytes | ( 0x000002 << 11 ); //traffic lights red, pedestrian green
					ShiftLED(buffer, bytes);
				}
				NextState = Transition;
			break;
		}
		State = NextState;
	}

}

