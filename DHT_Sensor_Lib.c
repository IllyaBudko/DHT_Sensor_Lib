#include "main.h"

/*
  DHT Sensor APIs  ====================================================================
*/

/*Function: DHT_OneWire_Init
  ---------------------------------------------------------
  Note:
  - Weak DHT sensor one wire(aosong 1-wire) initialization function, that inititializes the data line to GPIOA, GPIO_PIN_1;
  - Should be implemented in user file;
  
  Param:
   - hdht, DHT_Handle_t structure to be initialized
  
  Return Value: None
*/

__weak void DHT_OneWire_Init(DHT_Handle_t *hdht)
{
  hdht->dht_gpio_instance = GPIOA;
  hdht->dht_gpio_pin      = GPIO_PIN_1;
  hdht->dht_tim_instance  = TIM6;
  
  if(DHT_Init(hdht) != DHT_OK)
  {
    DHT_Error_Handler(hdht);
  }
}

/*Function: DHT_Error_Handler
  ---------------------------------------------------------
  Note:
  - Weak DHT error handler to handle errors according to the DHT_State_t enum in specific DHT_Handle_t
  - Should be implemented in user file;
  
  Param:
  - hdht, DHT_Handle_t structure for which to handle error
  
  Return Value: None
*/

__weak void DHT_Error_Handler(DHT_Handle_t *hdht)
{
  switch (DHT_Get_State(hdht))
  {
    case DHT_ERROR_Timeout:
      while(1);
      break;
    case DHT_ERROR_Response:
      while(1);
      break;
    case DHT_ERROR_Checksum:
      while(1);
      break;
    case DHT_ERROR_Init:
      while(1);
      break;
    default:
      while(1);
      break;
  }
}

/*Function: DHT_Read
  ---------------------------------------------------------
  Note:
  - Wrapper function that reads 40 bits of  data incoming from DHT sensor
  - Writes data back into DHT_Handle_t structure, in the form of a buffer, which then prepares a humidity array and a temperature array
  - Verifies checksum sent by DHT Sensor
  
  Param:
   - hdht, DHT_Handle_t structure to be used
  
  Return Value: None
*/

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

/*Function: DHT_Get_State
  ---------------------------------------------------------
  Note:
  - Gets DHT structure state
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: DHT_State_t enum value
*/

DHT_State_t DHT_Get_State(DHT_Handle_t *hdht)
{
  return hdht->dht_state;
}

/*
  DHT Helper function used inside APIs  ====================================================================
*/

/*Function: DHT_Init
  ---------------------------------------------------------
  Note:
  - Initializes gpio pin for use with DHT Sensor
  - Initializes timer to tick with 1uS interval
  - Starts timer
  
  Param:
  - hdht, DHT_Handle_t structure which contains initialization values
  
  Return Value: DHT_State_t enum value
*/

DHT_State_t DHT_Init(DHT_Handle_t *hdht)
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
    hdht->dht_state = DHT_ERROR_Init;
    Error_Handler();
  }
  
  if(HAL_TIM_Base_Start(&(hdht->dht_tim_handle)) != HAL_OK)
  {
    Error_Handler();
    hdht->dht_state = DHT_ERROR_Init;
  }
  
  return hdht->dht_state;
}

/*Function: DHT_uS_Delay
  ---------------------------------------------------------
  Note:
  - Delay function that ticks with 1uS
  
  Param: 
  - hdht, DHT_Handle_t structure to be used
  - uS_Delay, delay value in uS
  
  Return Value: None
*/

void DHT_uS_Delay(DHT_Handle_t *hdht, uint16_t uS_Delay)
{
  hdht->dht_tim_instance->CNT = 0;
  while(hdht->dht_tim_instance->CNT <= uS_Delay);
}

/*Function: DHT_Check_Timeout
  ---------------------------------------------------------
  Note:
  - Checks whether DHT sensor timesout while reading pin
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  - uSeconds, timeoout value in uSeconds
  
  Return Value: none
*/

void DHT_Check_Timeout(DHT_Handle_t *hdht,uint16_t uSeconds)
{
  DHT_uS_Delay(hdht,1);
  hdht->timeout++;
  if(hdht->timeout >= uSeconds)
  {
    hdht->dht_state = DHT_ERROR_Timeout;
    hdht->timeout = 0;
  }
}

/*Function: DHT_setInput
  ---------------------------------------------------------
  Note:
  - Sets GPIO pin as input
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: none
*/

void DHT_setInput(DHT_Handle_t *hdht)
{
  hdht->dht_gpio_init.Mode = GPIO_MODE_INPUT;
  hdht->dht_gpio_init.Pull = GPIO_NOPULL;
  hdht->dht_gpio_init.Pin  = hdht->dht_gpio_pin;
  
  HAL_GPIO_Init(hdht->dht_gpio_instance, &(hdht->dht_gpio_init));
}

/*Function: DHT_setOutput
  ---------------------------------------------------------
  Note:
  - Sets GPIO pin as output
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: none
*/

void DHT_setOutput(DHT_Handle_t *hdht)
{
  hdht->dht_gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  hdht->dht_gpio_init.Pull = GPIO_NOPULL;
  hdht->dht_gpio_init.Pin  = hdht->dht_gpio_pin;
  
  HAL_GPIO_Init(hdht->dht_gpio_instance, &(hdht->dht_gpio_init));
}



/*Function: Master_Transmit_Start
  ---------------------------------------------------------
  Note:
  - Microcontroller as master sends start sequence to DHT sensor to prepare data
  - Depending on DHT sensor the timings for the sequence varies, but in general
    the master data pin is set as output, then sends low for a few milliseconds
    then high for ~25uS, finally it switches to input and gives up control of the pin
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: none
*/

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
  if(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
    {
      DHT_Check_Timeout(hdht,500);
    }
  }
}

/*Function: Slave_Receive_Response
  ---------------------------------------------------------
  Note:
  - DHT sensor as slave responds to master that it has received the start signal
  - The responce sequence is ~80uS low then ~80uS high, after which it will send 40 bits of data
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: none
*/

void Slave_Receive_Response(DHT_Handle_t *hdht)
{
  if((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
  {
    while((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
    {
      DHT_Check_Timeout(hdht,500);
    }
  }
  else
  {
    hdht->dht_state = DHT_ERROR_Response;
  }
  
  if(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
  {
    while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
    {
      DHT_Check_Timeout(hdht,500);
    }
  }
  else
  {
    hdht->dht_state = DHT_ERROR_Response;
  }
}

/*Function: Byte_Read
  ---------------------------------------------------------
  Note:
  - DHT sensor as slave responds to master that it has received the start signal
  - The responce sequence is ~80uS low then ~80uS high, after which it will send 40 bits of data
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  - whichByte, which Byte in the 5byte(40 bits) is being decoded
  
  Return Value: none
*/

void Byte_Read(DHT_Handle_t *hdht, uint8_t whichByte)
{
  for(uint8_t j = 0; j < 8; j++)
  {
    //sensor pulls low for 50 uS so we need to wait it out
    while((!HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin))) && (hdht->dht_state) == DHT_OK)
    {
      DHT_Check_Timeout(hdht,500);
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
      while(HAL_GPIO_ReadPin((hdht->dht_gpio_instance),(hdht->dht_gpio_pin)) && (hdht->dht_state) == DHT_OK)
      {
        DHT_Check_Timeout(hdht,500);
      }
    }
  }
}

/*Function: Checksum_Verify
  ---------------------------------------------------------
  Note:
  - Verifies calculated checksum of first 4 bytes of buffer with sent DHT sensor checksum
  - If checksum is successful the it prepares humidity array and temperature array
  
  Param:
  - hdht, DHT_Handle_t structure to be used
  
  Return Value: none
*/

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
