/*
 * Trafficlight_functions.h
 *
 *  Created on: Nov 21, 2022
 *      Author: anton
 */

#ifndef INC_TRAFFICLIGHT_FUNCTIONS_H_
#define INC_TRAFFICLIGHT_FUNCTIONS_H_



#endif /* INC_TRAFFICLIGHT_FUNCTIONS_H_ */


void ShiftLED(uint32_t leds[], uint8_t bytesize);
void switchTL(void);
uint32_t setLights(uint8_t statebit);
uint32_t setPedestrian(uint8_t ped1, uint8_t ped2);
uint32_t readPed1(uint8_t ped1, uint32_t shift);
uint32_t readPed2(uint8_t ped2, uint32_t shift);
uint32_t removePedestrian(uint8_t ped1, uint8_t ped2);
