#include "main.h"

extern TIM_HandleTypeDef htim6;
extern GPIO_InitTypeDef dht_sensor;

void DHT_Raw_Read(uint8_t Data[4])
{
  uint8_t buffer[] = {0,0,0,0,0};
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t checksum = 0;
  
  // Set pin as output
  GPIO_setOutput(DHT_Port,DHT_Pin);
  
  // Write 0 to pin for 18 milliseconds to prepare sensor
  HAL_GPIO_WritePin(DHT_Port,DHT_Pin, GPIO_PIN_RESET);
  uS_Delay(18000);
  
  // Set pin as input
  GPIO_setInput(DHT_Port,DHT_Pin);
  
  /* since the data line is pulled up externally we need to wait
     for the sensor to take over the line after the pin is set as input
  */
  while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  
  //DHT sensor first responds with low for 80uS then high for 80uS we wait those out
  while(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
  
  //1 wire from sensor decode starts here
  
  //cycle for every buffer byte
  for(i = 0; i <= 5; i++)
  {
    // cycle for every bit
    for(j = 8; j > 0; j--)
    {
      //sensor pulls low for 50 uS so we need to wait it out
      while(! HAL_GPIO_ReadPin(DHT_Port,DHT_Pin));
      uS_Delay(35);
      if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin))
      {
        buffer[i] |= (1 << (j - 1));
      }
      else
      {
        buffer[i] &= ~(1 << (j - 1)); 
      }                                                      
    }                                                        
  }                                                          
  //checksum here for status
  
  Data[0] = buffer[0];
  Data[1] = buffer[1];
  Data[2] = buffer[2];
  Data[3] = buffer[3];
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
