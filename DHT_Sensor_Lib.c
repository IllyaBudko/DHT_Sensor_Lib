#include "main.h"

extern TIM_HandleTypeDef htim6;
extern GPIO_InitTypeDef dht_sensor;

DHT_State_t DHT_Raw_Read(uint8_t Data[4])
{
  uint8_t buffer[] = {0,0,0,0,0};
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t cnt = 0;
  uint16_t timeout = 0;
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
  timeout = 0;
  if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
  {
    while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
    {
      uS_Delay(1);
      timeout++;
      if(timeout >= 1000)
      {
        DHT_State = DHT_ERROR_Timeout;
      }
    }
  }
  /******************************************************RESPONSE **********************************************************************/
  //DHT sensor first responds with low for 80uS then high for 80uS we wait those out
  if((!HAL_GPIO_ReadPin(DHT_Port,DHT_Pin)) && DHT_State == DHT_OK)
  {
    // removed delay, using timeout as timing :S
    timeout = 0;
    while((!HAL_GPIO_ReadPin(DHT_Port,DHT_Pin)) && DHT_State == DHT_OK)
    {
      uS_Delay(1);
      timeout++;
      if(timeout >= 1000)
      {
        DHT_State = DHT_ERROR_Timeout;
      }
    }
  }
  else
  {
    DHT_State = DHT_ERROR_Response;
  }
  
  timeout = 0;
  if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
  {
    while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
    {
      uS_Delay(1);
      timeout++;
      if(timeout >= 1000)
      {
        DHT_State = DHT_ERROR_Timeout;
      }
    }
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
        timeout = 0;
        while((!HAL_GPIO_ReadPin(DHT_Port,DHT_Pin)) && DHT_State == DHT_OK)
        {
          uS_Delay(1);
          timeout++;
          if(timeout >= 1000)
          {
            DHT_State = DHT_ERROR_Timeout;
          }
        }
        uS_Delay(28);
        if((!HAL_GPIO_ReadPin(DHT_Port,DHT_Pin)) && DHT_State == DHT_OK)
        {
          buffer[i] &= ~(1 << (7 - j));
        }
        else if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
        {
          buffer[i] |= (1 << (7 - j));
          
          // cnt is used to not be stuck in infinite loop on the last bit read
          if(cnt < 40)
          {
            //important line to skip the rest of the 70uS of "1" bit
            timeout = 0;
            while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
            {
              uS_Delay(1);
              timeout++;
              if(timeout >= 1000)
              {
                DHT_State = DHT_ERROR_Timeout;
              }
            }
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

__weak void DHT_Error_Handler(DHT_State_t State)
{
  switch (State)
  {
    case DHT_ERROR_Timeout:
      break;
    case DHT_ERROR_Response:
      break;
    case DHT_ERROR_Checksum:
      break;
    default:
      break;
  }
}

void uS_Delay(uint16_t uSeconds,TIM_HandleTypeDef dht_tim)
{
  dht_tim.Instance->CR1 |= (1 << 0);
  __HAL_TIM_SET_COUNTER(&dht_tim,0);
  while(__HAL_TIM_GET_COUNTER(&dht_tim) < uSeconds);
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






