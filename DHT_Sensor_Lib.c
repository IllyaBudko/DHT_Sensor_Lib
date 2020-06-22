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

void DHT_Init(DHT_Handle_t *dht_handle)
{
  uint32_t HCLK_freq;
  HCLK_freq = HAL_RCC_GetHCLKFreq();
  //1. Setup GPIO Pin for data acquisition
  dht_handle->dht_input_init.Mode = GPIO_MODE_OUTPUT_OD;
  dht_handle->dht_input_init.Pull = GPIO_NOPULL;
  dht_handle->dht_input_init.Pin  = dht_handle->dht_input_pin;
  
  HAL_GPIO_Init(dht_handle->dht_input_instance, &(dht_handle->dht_input_init));
  
  //2. Setup TIM for uS delay
  dht_handle->dht_tim_handle.Instance                = dht_handle->dht_tim_instance;
  dht_handle->dht_tim_handle.Init.Prescaler          = (HCLK_freq/1000000) - 1;
  dht_handle->dht_tim_handle.Init.CounterMode        = TIM_COUNTERMODE_UP;
  dht_handle->dht_tim_handle.Init.Period             = 0xFFFF-1;
  dht_handle->dht_tim_handle.Init.AutoReloadPreload  = TIM_AUTORELOAD_PRELOAD_DISABLE;
  
  if(HAL_TIM_Base_Init(&(dht_handle->dht_tim_handle)) != HAL_OK)
  {
    Error_Handler();
  }
  
  if(HAL_TIM_Base_Start(&(dht_handle->dht_tim_handle)) != HAL_OK)
  {
    Error_Handler();
  }
}

void DHT_Read(DHT_Handle_t *dht_handle)
{
  uint8_t i = 0;
  //Master send start
  Master_Transmit_Start(dht_handle);
  //Receive slave response
  Slave_Receive_Response(dht_handle);
  //Decode receive
  if(dht_handle->dht_state == DHT_OK)
  {
    //cycle for every buffer byte
    for(i = 0; i < 5; i++)
    {
      Byte_Read(dht_handle,i);
    }      
  }
  //Verify checksum
  Checksum_Verify(dht_handle);
  //...
}

void DHT_uS_Delay(DHT_Handle_t *dht_handle, uint16_t uS_Delay)
{
  /*
  dht_handle.dht_tim_handle.Instance->CR1 |= (1 << 0);
  __HAL_TIM_SET_COUNTER(&(dht_handle.dht_tim_handle),0);
  while(__HAL_TIM_GET_COUNTER(&(dht_handle.dht_tim_handle)) < uS_Delay);
  */
  dht_handle->dht_tim_instance->CNT = 0;
  while(dht_handle->dht_tim_instance->CNT <= uS_Delay);
}

/////////////////////// Helper Functions ////////////////////////////
void DHT_setInput(DHT_Handle_t *dht_handle)
{
  dht_handle->dht_input_init.Mode = GPIO_MODE_INPUT;
  dht_handle->dht_input_init.Pull = GPIO_NOPULL;
  dht_handle->dht_input_init.Pin  = dht_handle->dht_input_pin;
  
  HAL_GPIO_Init(dht_handle->dht_input_instance, &(dht_handle->dht_input_init));
}
void DHT_setOutput(DHT_Handle_t *dht_handle)
{
  dht_handle->dht_input_init.Mode = GPIO_MODE_OUTPUT_OD;
  dht_handle->dht_input_init.Pull = GPIO_NOPULL;
  dht_handle->dht_input_init.Pin  = dht_handle->dht_input_pin;
  
  HAL_GPIO_Init(dht_handle->dht_input_instance, &(dht_handle->dht_input_init));
}

void Master_Transmit_Start(DHT_Handle_t *dht_handle)
{
  DHT_setOutput(dht_handle);
    // Write 0 to pin for 18 milliseconds to prepare sensor
  HAL_GPIO_WritePin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin), GPIO_PIN_RESET);
  DHT_uS_Delay(dht_handle,18000);

  
  // Set pin as input
  DHT_setInput(dht_handle);
  
  /* 
    since the data line is pulled up externally we need to wait
    for the sensor to take over the line after the pin is set as input
  */
  dht_handle->timeout = 0;
  if(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(dht_handle,2);
      dht_handle->timeout++;
      if(dht_handle->timeout >= 500)
      {
        dht_handle->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
}
void Slave_Receive_Response(DHT_Handle_t *dht_handle)
{
  if((!HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin))) && (dht_handle->dht_state) == DHT_OK)
  {
    // removed delay, using timeout as timing :S
    dht_handle->timeout = 0;
    while((!HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin))) && (dht_handle->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(dht_handle,2);
      dht_handle->timeout++;
      if(dht_handle->timeout >= 1000)
      {
        dht_handle->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
  else
  {
    dht_handle->dht_state = DHT_ERROR_Response;
  }
  
  dht_handle->timeout = 0;
  if(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(dht_handle,2);
      dht_handle->timeout++;
      if(dht_handle->timeout >= 1000)
      {
        dht_handle->dht_state = DHT_ERROR_Timeout;
      }
    }
  }
  else
  {
    dht_handle->dht_state = DHT_ERROR_Response;
  }
}

void Byte_Read(DHT_Handle_t *dht_handle, uint8_t whichByte)
{
  uint8_t j = 0;
  uint8_t cnt = 0;
  for(j = 0; j < 8; j++)
  {
    cnt++;
    //sensor pulls low for 50 uS so we need to wait it out
    dht_handle->timeout = 0;
    while((!HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin))) && (dht_handle->dht_state) == DHT_OK)
    {
      DHT_uS_Delay(dht_handle,2);
      dht_handle->timeout++;
      if(dht_handle->timeout >= 500)
      {
        dht_handle->dht_state = DHT_ERROR_Timeout;
      }
    }
    DHT_uS_Delay(dht_handle,28);
    if((!HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin))) && (dht_handle->dht_state) == DHT_OK)
    {
      dht_handle->buffer[whichByte] &= ~(1 << (7 - j));
    }
    else if(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
    {
      dht_handle->buffer[whichByte] |= (1 << (7 - j));
      
      // cnt is used to not be stuck in infinite loop on the last bit read
      if(cnt < 41)
      {
        //important line to skip the rest of the 70uS of "1" bit
        dht_handle->timeout = 0;
        while(HAL_GPIO_ReadPin((dht_handle->dht_input_instance),(dht_handle->dht_input_pin)) && (dht_handle->dht_state) == DHT_OK)
        {
          DHT_uS_Delay(dht_handle,2);
          dht_handle->timeout++;
          if(dht_handle->timeout >= 500)
          {
            dht_handle->dht_state = DHT_ERROR_Timeout;
          }
        }
      }
    }
  }
}

void Checksum_Verify(DHT_Handle_t *dht_handle)
{
  if(dht_handle->dht_state == DHT_OK)
  {
    dht_handle->sent_checksum = dht_handle->buffer[4];
    uint8_t data_checksum = dht_handle->buffer[0] + dht_handle->buffer[1] + dht_handle->buffer[2] + dht_handle->buffer[3];
    if(dht_handle->sent_checksum == data_checksum)
    {
      dht_handle->humidity[0] = dht_handle->buffer[0];
      dht_handle->humidity[1] = dht_handle->buffer[1];
      dht_handle->temperature[0] = dht_handle->buffer[2];
      dht_handle->temperature[1] = dht_handle->buffer[3];
    }
    else
    {
      dht_handle->dht_state = DHT_ERROR_Checksum;
    }
  }
}

////////////////////////// Version 2.0 //////////////////////////////







