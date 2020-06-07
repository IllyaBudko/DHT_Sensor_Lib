#include "main.h"

extern TIM_HandleTypeDef htim6;
extern GPIO_InitTypeDef dht_sensor;

DHT_State_t DHT_Raw_Read(uint8_t Data[4])
{
  uint8_t buffer[] = {0,0,0,0,0};
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t cnt = 0;
  uint8_t checksum = 0;
  
  DHT_State_t DHT_State = DHT_OK;
  
  /******************************************************MASTER START SIGNAL **********************************************************************/
  // Set pin as output
  GPIO_setOutput(DHT_Port,DHT_Pin);
  
  // Write 0 to pin for 18 milliseconds to prepare sensor
  HAL_GPIO_WritePin(DHT_Port,DHT_Pin, GPIO_PIN_RESET);
  uS_Delay(18000);

  
  // Set pin as input
  GPIO_setInput(DHT_Port,DHT_Pin);
  
  /* 
    since the data line is pulled up externally we need to wait
    for the sensor to take over the line after the pin is set as input
  */
  while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  /******************************************************RESPONSE **********************************************************************/
  //DHT sensor first responds with low for 80uS then high for 80uS we wait those out
  uS_Delay(40);
  if(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) || DHT_State == DHT_OK)
  {
    while(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  }
  else
  {
    DHT_State = DHT_ERROR_Response;
  }
  if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) || DHT_State == DHT_OK)
  {
    while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  }
  else
  {
    DHT_State = DHT_ERROR_Response;
  }
  
  /******************************************************DECODE **********************************************************************/
  if(DHT_State == DHT_OK)
  {
    //cycle for every buffer byte
    for(i = 0; i < 6; i++)
    {
      // cycle for every bit
      for(j = 0; j < 8; j++)
      {
        cnt++;
        //sensor pulls low for 50 uS so we need to wait it out
        while(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
        uS_Delay(28);
        if(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin))
        {
          buffer[i] &= ~(1 << (7 - j));
        }
        else
        {
          buffer[i] |= (1 << (7 - j));
          
          // cnt is used to not be stuck in infinite loop on the last bit read
          if(cnt < 40)
          {
            //important line to skip the rest of the 70uS of "1" bit
            while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
          }
        }
      }   
    }
  }
  
  /******************************************************CHECKSUM **********************************************************************/
  checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
  if(checksum == buffer[4])
  {
    Data[0] = buffer[0];
    Data[1] = buffer[1];
    Data[2] = buffer[2];
    Data[3] = buffer[3];
  }
  else
  {
    DHT_State = DHT_ERROR_Checksum;
  }
    /******************************************************STATE RETURN **********************************************************************/
  return DHT_State;
}


void uS_Delay(uint16_t uSeconds)
{
  TIM6->CR1 |= (1 << 0);
  __HAL_TIM_SET_COUNTER(&htim6,0);
  while(__HAL_TIM_GET_COUNTER(&htim6) < uSeconds);
}


void GPIO_setInput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
  dht_sensor.Mode = GPIO_MODE_INPUT;
  dht_sensor.Pull = GPIO_NOPULL;
  dht_sensor.Pin  = GPIO_Pin;
  
  HAL_GPIO_Init(GPIOx, &dht_sensor);
  
}

void GPIO_setOutput(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
  dht_sensor.Mode = GPIO_MODE_OUTPUT_OD;
  dht_sensor.Pull = GPIO_NOPULL;
  dht_sensor.Pin  = GPIO_Pin;
  
  HAL_GPIO_Init(GPIOx, &dht_sensor);
  
}
