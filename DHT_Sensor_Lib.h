#ifndef  __DHT_SENSOR_LIB_H
#define  __DHT_SENSOR_LIB_H

#include <stdio.h>
#include <stdint.h>
#include "main.h"

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
  GPIO_InitTypeDef    dht_gpio_init;
  GPIO_TypeDef       *dht_gpio_instance;
  uint32_t            dht_gpio_pin;
  DHT_State_t         dht_state;
  uint16_t            timeout;
  uint8_t             buffer[5];
  uint8_t             humidity[2];                //[0] integer humidity, [1] decimal humidity
  uint8_t             temperature[2];             //[0] integer temperature, [1] decimal temperature
  uint8_t             sent_checksum;
  
}DHT_Handle_t;

/*
  DHT Sensor APIs  ====================================================================
*/

__weak void DHT_OneWire_Init(DHT_Handle_t *hdht);
__weak void DHT_Error_Handler(DHT_Handle_t *hdht);

void DHT_Read(DHT_Handle_t *hdht);

DHT_State_t DHT_Get_State(DHT_Handle_t *hdht);

/*
  DHT Helper function used inside APIs  ====================================================================
*/

DHT_State_t DHT_Init(DHT_Handle_t *hdht);

void DHT_uS_Delay(DHT_Handle_t *hdht, uint16_t uS_Delay);
void DHT_Check_Timeout(DHT_Handle_t *hdht,uint16_t uSeconds);

void DHT_setInput(DHT_Handle_t *hdht);
void DHT_setOutput(DHT_Handle_t *hdht);

void Master_Transmit_Start(DHT_Handle_t *hdht);
void Slave_Receive_Response(DHT_Handle_t *hdht);
void Byte_Read(DHT_Handle_t *hdht, uint8_t whichByte);
void Checksum_Verify(DHT_Handle_t *hdht);

#endif /*__DHT_SENSOR_LIB_H*/
