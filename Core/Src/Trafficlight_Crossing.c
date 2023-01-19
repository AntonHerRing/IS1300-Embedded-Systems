/**
******************************************************************************
@brief 		Trafficlight Crossing, program simulating a 4-way crossing with
			a pedestrian crossing
@file 		Trafficlight_Crossing.c
@author 		Anton Herdin Ringstedt
@version 1.0
@date 		13-December-2022
@brief 		The program containing Traffic light Crossing state-machine
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stdbool.h"
#include "Test.h"
#include "Trafficlight_functions.h"
#include "Crossing_functions.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/**
 @variables	struct containing state machine states
 */
typedef enum
{ Base,
Transition,
Go,
Lane1,
Lane2
}states;

static states State, NextState;

/**
 @variable1	time constant specific variable
 */
uint16_t ToggleFreq = 500;
/**
 @variable2	time constant specific variable
 */
uint16_t PedestrianDelay = 6000;
/**
@variable3	time constant specific variable
 */
uint16_t WalkingDelay = 7000;
/**
 @variable4	time constant specific variable
 */
uint16_t OrangeDelay = 2000;
/**
  @variable5	time constant specific variable
 */
uint16_t GreenDelay = 5000;
/**
@variable6	time constant specific variable
*/
uint16_t RedDelayMax = 10000;

/**
 @variable1	number of bytes for shift register
 */
uint8_t bytes = 3;
/**
 @variable2	buffer for shift register
 */
uint32_t buffer[]= {0x000000};
/**
 @variable3	temporary bit holder for shift register
 */
uint32_t shiftbytes = 0x000000;

/**
 @variable1	current active pedestrian variable
 */
bool ped1 = false;
/**
 @variable2	current active pedestrian variable
 */
bool ped2 = false;

/**
 @variable1	time keeping specific variable
 */
uint32_t time = 0;

/**
 @variable2	time keeping specific variable
 */
uint32_t togtime = 0;
/**
 @variable3	time keeping specific variable
 */
uint32_t pedtime = 0;

/**
 @variables	keep track of last state: 0 = last state base, 1 = last state go
 */
uint8_t lastState = 0;

/**
@brief 		PedestrianToggle, handles pedestrian activation and
 	 	 	toggling of blue LED
@param1 	void, no parameter
@return 	void, no return value
*/
void PedestrianToggle(void)
{
	//check if pedestrian active
	if(HAL_GPIO_ReadPin(GPIOA ,PL1_Switch_Pin) == GPIO_PIN_RESET && NextState != Transition)
	{
		NextState = Transition;
		togtime = HAL_GetTick();
		pedtime = HAL_GetTick();
		ped1 = false;
		ped2 = true;
	}
	else if(HAL_GPIO_ReadPin(GPIOB ,PL2_Switch_Pin) == GPIO_PIN_RESET && NextState != Transition)
	{
		NextState = Transition;
		togtime = HAL_GetTick();
		pedtime = HAL_GetTick();
		ped2 = false;
		ped1 = true;
	}

	if((HAL_GetTick() - togtime) == ToggleFreq && NextState == Transition)
	{
		//traffic lights green, pedestrian toggle off
		shiftbytes = setPedestrian(1, 1);
		buffer[0] |= shiftbytes;
		ShiftLED(buffer, bytes);
	}
	else if((HAL_GetTick() - togtime) == ToggleFreq * 2  && NextState == Transition)
	{
		// pedestrian toggle on
		if(ped1)
			shiftbytes = setPedestrian(4|1, 1);
		else if(ped2)
			shiftbytes = setPedestrian(1, 4|1);
		togtime = HAL_GetTick();
		buffer[0] |= shiftbytes;
		ShiftLED(buffer, bytes);
	}
	else if(NextState != Transition)
		shiftbytes = setPedestrian(1, 1);
}

/**
@brief 		Trafficlight_Crossing, state machine for program itself
@param1 	void, no parameter
@return 	void, no return value
*/
void Trafficlight_Crossing(void)
{
	State = Base;
	buffer[0] = setTraffic(4,1);
	ShiftLED(buffer, bytes);

	while(1)
	{
		switch(State)
		{
			case Base:
				time = HAL_GetTick();
				while(1) //stay while no cars
				{
					//traffic check
					if((HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET) && NextState != Transition)
					{
						NextState = Lane2;
						break;
					}
					else if((HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET) && NextState != Transition)
					{
						NextState = Lane1;
						break;
					}

					//traffic activation
					if((HAL_GetTick() - time) == (GreenDelay - OrangeDelay) || (HAL_GetTick() - time) == (GreenDelay * 2 - OrangeDelay))
						buffer[0] = setTraffic(2,2); 	//both lanes orange

					if((HAL_GetTick() - time) == GreenDelay )
					{
						buffer[0] = setTraffic(4,1); 	//lane1 green, lane2 red
					}
					else if((HAL_GetTick() - time) >= GreenDelay*2 )
					{
						buffer[0] = setTraffic(1,4) ; 	//lane1 red, lane2 green
						time = HAL_GetTick();
					}

					//pedestrian activation, clean buffer from  previous shiftbyte
					buffer[0] &= ~shiftbytes;
					PedestrianToggle();

					buffer[0] |= shiftbytes;
					ShiftLED(buffer, bytes);

					if((HAL_GetTick() - pedtime) == (PedestrianDelay - OrangeDelay) && NextState == Transition)
						break;
				}
				lastState = 0;
			break;

			case Lane1:
				NextState = Base;
				time = HAL_GetTick();
				while((HAL_GetTick() - time) <= RedDelayMax || NextState == Transition)	//stay while lane2 no cars
				{
					//break if no cars in lane
					if(HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_RESET)
						break;

					//pedestrian activation
					PedestrianToggle();

					//transition orange
					if((HAL_GetTick() - time) <= OrangeDelay && !readLane1(4, buffer[0]))
						buffer[0] = setTraffic(2,2);
					else
						buffer[0] = setTraffic(4,1); 	//lane1 green, lane2 red
					buffer[0] |= shiftbytes;	//pedestrian red
					ShiftLED(buffer, bytes);

					if((HAL_GetTick() - pedtime) == (PedestrianDelay - OrangeDelay) && NextState == Transition)
						break;
				}
				//change lane
				if((HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_SET) && NextState != Transition)
					NextState = Lane2;

			break;

			case Lane2:
				NextState = Base;
				time = HAL_GetTick();
				while((HAL_GetTick() - time) <= RedDelayMax || NextState == Transition)	//stay while lane1 no cars
				{
					//break if no cars in lane
					if(HAL_GPIO_ReadPin(TL1_Car_GPIO_Port ,TL1_Car_Pin) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOB ,TL3_Car_Pin) == GPIO_PIN_RESET)
						break;

					//pedestrian activation
					PedestrianToggle();

					//transition orange
					if((HAL_GetTick() - time) <= OrangeDelay && !readLane2(4, buffer[0]))
						buffer[0] = setTraffic(2,2);
					else
						buffer[0] = setTraffic(1,4); 	//lane1 red, lane2 green
					buffer[0] |= shiftbytes;	//pedestrian red
					ShiftLED(buffer, bytes);

					if((HAL_GetTick() - pedtime) == (PedestrianDelay - OrangeDelay) && NextState == Transition)
						break;
				}
				//change lane
				if((HAL_GPIO_ReadPin(GPIOB ,TL2_Car_Pin) == GPIO_PIN_SET || HAL_GPIO_ReadPin(GPIOA ,TL4_Car_Pin) == GPIO_PIN_SET) && NextState != Transition)
					NextState = Lane1;
			break;

			case Transition:
				time = HAL_GetTick();
				while((HAL_GetTick() - time) <= OrangeDelay) // orange transition
				{
					if((HAL_GetTick() - togtime) == ToggleFreq || lastState == 1)
						buffer[0] = setTraffic(2,2) | setPedestrian(1, 1); //traffic lights orange, pedestrian toggle off
					else if(HAL_GetTick() - togtime == ToggleFreq * 2 || (HAL_GetTick() - togtime) > ToggleFreq * 2 )
					{
						if(ped1)
							buffer[0] = setTraffic(2,2) | setPedestrian(4|1, 1); //traffic lights orange, pedestrian toggle on
						else if(ped2)
							buffer[0] = setTraffic(2,2) | setPedestrian(1, 4|1);
						togtime = HAL_GetTick();
					}
					ShiftLED(buffer, bytes);
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
					if(ped1)
						buffer[0] = setTraffic(1,4) | setPedestrian(2, 1); //traffic lights red, pedestrian green
					else if(ped2)
						buffer[0] = setTraffic(4,1) | setPedestrian(1, 2);
					ShiftLED(buffer, bytes);
				}
				NextState = Transition;
			break;
		}
		State = NextState;
	}
}

