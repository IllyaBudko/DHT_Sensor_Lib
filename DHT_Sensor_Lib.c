#include "main.h"

extern TIM_HandleTypeDef htim6;
extern GPIO_InitTypeDef dht_sensor;

////////////////////////// Version 1.0 //////////////////////////////

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
  uS_Delay(18000,htim6);

  
  // Set pin as gpio
  GPIO_setInput(DHT_Port,DHT_Pin);
  
  /* 
    since the data line is pulled up externally we need to wait
    for the sensor to take over the line after the pin is set as gpio
  */
  timeout = 0;
  if(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
  {
    while(HAL_GPIO_ReadPin(DHT_Port,DHT_Pin) && DHT_State == DHT_OK)
    {
      uS_Delay(1,htim6);
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
      uS_Delay(1,htim6);
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
      uS_Delay(1,htim6);
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
    for(i = 0; i < 5; i++)
    {
      // cycle for every bit
      for(j = 0; j < 8; j++)
      {
        cnt++;
        //sensor pulls low for 50 uS so we need to wait it out
        timeout = 0;
        while((!HAL_GPIO_ReadPin(DHT_Port,DHT_Pin)) && DHT_State == DHT_OK)
        {
          uS_Delay(1,htim6);
          timeout++;
          if(timeout >= 1000)
          {
            DHT_State = DHT_ERROR_Timeout;
          }
        }
        uS_Delay(28,htim6);
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
              uS_Delay(1,htim6);
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
    case DHT_ERROR_Init:
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
////////////////////////// Version 1.0 //////////////////////////////

////////////////////////// Version 2.0 //////////////////////////////

__weak void DHT_Initialization(DHT_Handle_t *hdht)
{
  
}

void DHT_Init(DHT_Handle_t *hdht)
{
  uint32_t HCLK_freq;
  HCLK_freq = HAL_RCC_GetHCLKFreq();
  //1. Setup GPIO Pin for data acquisition
  hdht->dht_gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  hdht->dht_gpio_init.Pull = GPIO_NOPULL;
  hdht->dht_gpio_init.Pin  = hdht->dht_gpio_pin;
  
  HAL_GPIO_Init(hdht->dht_gpio_instance, &(hdht->dht_gpio_init));
  
  //2. Setup TIM for uS delay
  hdht->dht_tim_handle.Instance                = hdht->dht_tim_instance;
  hdht->dht_tim_handle.Init.Prescaler          = (HCLK_freq/1000000) - 1;
  hdht->dht_tim_handle.Init.CounterMode        = TIM_COUNTERMODE_UP;
  hdht->dht_tim_handle.Init.Period             = 0xFFFF-1;
  hdht->dht_tim_handle.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  if(HAL_TIM_Base_Init(&(hdht->dht_tim_handle)) != HAL_OK)
  {
    Error_Handler();
  }
  
  if(HAL_TIM_Base_Start(&(hdht->dht_tim_handle)) != HAL_OK)
  {
    Error_Handler();
  }
}

void DHT_Read(DHT_Handle_t *hdht)
{
  uint8_t i = 0;
  //Master send start
  Master_Transmit_Start(hdht);
  //Receive slave response
  Slave_Receive_Response(hdht);
  //Decode receive
  if(hdht->dht_state == DHT_OK)
  {
    //cycle for every buffer byte
    for(i = 0; i < 5; i++)
    {
      Byte_Read(hdht,i);
    }      
  }
  //Verify checksum
  Checksum_Verify(hdht);
}

void DHT_uS_Delay(DHT_Handle_t *hdht, uint16_t uS_Delay)
{
  hdht->dht_tim_instance->CNT = 0;
  while(hdht->dht_tim_instance->CNT <= uS_Delay);
}

DHT_State_t DHT_Get_State(DHT_Handle_t *hdht)
{
  return hdht->dht_state;
}

/////////////////////// Helper Functions ////////////////////////////
void DHT_setInput(DHT_Handle_t *hdht)
{
  hdht->dht_gpio_init.Mode = GPIO_MODE_INPUT;
  hdht->dht_gpio_init.Pull = GPIO_NOPULL;
  hdht->dht_gpio_init.Pin  = hdht->dht_gpio_pin;
  
  HAL_GPIO_Init(hdht->dht_gpio_instance, &(hdht->dht_gpio_init));
}
void DHT_setOutput(DHT_Handle_t *hdht)
{
  hdht->dht_gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  hdht->dht_gpio_init.Pull = GPIO_NOPULL;
  hdht->dht_gpio_init.Pin  = hdht->dht_gpio_pin;
  
  HAL_GPIO_Init(hdht->dht_gpio_instance, &(hdht->dht_gpio_init));
}

void DHT_Check_Timeout(DHT_Handle_t *hdht,uint16_t uSeconds)
{
  //Needs implementation //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void Master_Transmit_Start(DHT_Handle_t *hdht)
{
  DHT_setOutput(hdht);
    // Write 0 to pin for 18 milliseconds to prepare sensor
  HAL_GPIO_WritePin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin), GPIO_PIN_RESET);
  DHT_uS_Delay(hdht,18000);

  
  // Set pin as gpio
  DHT_setInput(hdht);
  
  /* 
    since the data line is pulled up externally we need to wait
    for the sensor to take over the line after the pin is set as gpio
  */
  hdht->timeout = 0;
  if(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(hdht,2);
      hdht->timeout++;
      if(hdht->timeout >= 500)
      {
        hdht->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
}
void Slave_Receive_Response(DHT_Handle_t *hdht)
{
  if((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
  {
    // removed delay, using timeout as timing :S
    hdht->timeout = 0;
    while((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(hdht,2);
      hdht->timeout++;
      if(hdht->timeout >= 1000)
      {
        hdht->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
  else
  {
    hdht->dht_state = DHT_ERROR_Response;
  }
  
  hdht->timeout = 0;
  if(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(hdht,2);
      hdht->timeout++;
      if(hdht->timeout >= 1000)
      {
        hdht->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
  else
  {
    hdht->dht_state = DHT_ERROR_Response;
  }
}

void Byte_Read(DHT_Handle_t *hdht, uint8_t whichByte)
{
  uint8_t j = 0;
  for(j = 0; j < 8; j++)
  {
    //sensor pulls low for 50 uS so we need to wait it out
    hdht->timeout = 0;
    while((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(hdht,2);
      hdht->timeout++;
      if(hdht->timeout >= 500)
      {
        hdht->dht_state = DHT_ERROR_Timeout;
      }
    }
    DHT_uS_Delay(hdht,28);
    if((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
    {
      hdht->buffer[whichByte] &= ~(1 << (7 - j));
    }
    else if(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
    {
      hdht->buffer[whichByte] |= (1 << (7 - j));

      //important line to skip the rest of the 70uS of "1" bit
      hdht->timeout = 0;
      while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
      {
        DHT_uS_Delay(hdht,2);
        hdht->timeout++;
        if(hdht->timeout >= 500)
        {
          hdht->dht_state = DHT_ERROR_Timeout;
        }
      }
    }
  }
}

void Checksum_Verify(DHT_Handle_t *hdht)
{
  if(hdht->dht_state == DHT_OK)
  {
    hdht->sent_checksum = hdht->buffer[4];
    uint8_t data_checksum = hdht->buffer[0] + hdht->buffer[1] + hdht->buffer[2] + hdht->buffer[3];
    if(hdht->sent_checksum == data_checksum)
    {
      hdht->humidity[0] = hdht->buffer[0];
      hdht->humidity[1] = hdht->buffer[1];
      hdht->temperature[0] = hdht->buffer[2];
      hdht->temperature[1] = hdht->buffer[3];
    }
    else
    {
      hdht->dht_state = DHT_ERROR_Checksum;
    }
  }
}

////////////////////////// Version 2.0 //////////////////////////////







