#ifndef  __DHT_SENSOR_LIB_H
#define  __DHT_SENSOR_LIB_H

#include <stdio.h>
#include <stdint.h>
#include "main.h"

#define DHT_Port    GPIOA
#define DHT_Pin     (uint16_t)GPIO_PIN_1

/*Fix 1*/

typedef enum
{
  DHT_OK,
  DHT_ERROR_Response,
  DHT_ERROR_Timeout,
  DHT_ERROR_Checksum
}DHT_State_t;

DHT_State_t DHT_Raw_Read(uint8_t Data[4]);
__weak void DHT_Error_Handler(DHT_State_t State);

void uS_Delay(uint16_t uSeconds);
void GPIO_setInput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
void GPIO_setOutput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);

#endif /*__DHT_SENSOR_LIB_H*/
