#ifndef  __DHT_SENSOR_LIB_H
#define  __DHT_SENSOR_LIB_H

#include <stdio.h>
#include <stdint.h>
#include "main.h"

#define DHT_Port    GPIOA
#define DHT_Pin     (uint16_t)GPIO_PIN_1

#define uMilliSec   (uint8_t)0
#define uMicroSec   (uint8_t)1

/*Fix 1*/

typedef enum
{
  DHT_OK,
  DHT_ERROR_Response,
  DHT_ERROR_Timeout,
  DHT_ERROR_Checksum,
  DHT_ERROR_Init
}DHT_State_t;

typedef struct
{
  TIM_HandleTypeDef   dht_tim_handle;
  TIM_TypeDef        *dht_tim_instance;
  GPIO_InitTypeDef    dht_input_init;
  GPIO_TypeDef       *dht_input_instance;
  uint32_t            dht_input_pin;
  DHT_State_t         dht_state;
  uint16_t            timeout;
  uint8_t             buffer[5];
  uint8_t             humidity[2];           //[0] integer humidity, [1] decimal humidity
  uint8_t             temperature[2];        //[0] integer temperature, [1] decimal temperature
  uint8_t             sent_checksum;
  
}DHT_Handle_t;

////////////////////////// Version 1.0 //////////////////////////////
DHT_State_t DHT_Raw_Read(uint8_t Data[4]);
__weak void DHT_Error_Handler(DHT_State_t State);

void uS_Delay(uint16_t uSeconds,TIM_HandleTypeDef dht_tim);
void GPIO_setInput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
void GPIO_setOutput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
////////////////////////// Version 1.0 //////////////////////////////

////////////////////////// Version 2.0 //////////////////////////////

void DHT_Init(DHT_Handle_t dht_handle);
void DHT_Read(DHT_Handle_t dht_handle);
void DHT_uS_Delay(DHT_Handle_t dht_handle, uint16_t uS_Delay);


/////////////////////// Helper Functions ////////////////////////////
void DHT_setInput(DHT_Handle_t dht_handle);
void DHT_setOutput(DHT_Handle_t dht_handle);

void Master_Transmit_Start(DHT_Handle_t dht_handle);
void Slave_Receive_Response(DHT_Handle_t dht_handle);
void Byte_Read(DHT_Handle_t dht_handle, uint8_t whichByte);
void Checksum_Verify(DHT_Handle_t dht_handle);

////////////////////////// Version 2.0 //////////////////////////////
#endif /*__DHT_SENSOR_LIB_H*/
