/*!
 * \file      main.c
 *
 * \brief     Ping-Pong implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   this is the main!
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "display.h"
#include "main_master.h"

#include "onewire.h"
#include "ds18b20.h"
#include "tim.h"

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;


static uint8_t 	app_request =0;
static int8_t 	indoor_temp = 0;
static int8_t 	outdoor_temp = 0;
static int8_t 	water_temp = 0;


int8_t RssiValue = 0;
int8_t SnrValue = 0;

static int8_t read_temp(void);

/* Led Timers objects*/
static  TimerEvent_t timerLed;

/* Temp read Timers objects*/
static  TimerEvent_t timerTemp;

/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function executed on when led timer elapses
 */
static void OnledEvent(void *context);
static void OnTempEvent(void *context);

/**
 * Main application entry point.
 */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  //DBG_Init();

  HW_Init();

  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

  /* Led Timers*/
  TimerInit(&timerLed, OnledEvent);
  TimerSetValue(&timerLed, LED_PERIOD_MS);
	
	  /* Led Timers*/
  TimerInit(&timerTemp, OnTempEvent);
  TimerSetValue(&timerTemp, TEMP_PERIOD_MS);
	TimerStart(&timerTemp);


  // Radio initialization
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);


  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);


	MX_TIM22_Init();
 HAL_TIM_Base_Start(&htim22);
 DS18B20_Init(DS18B20_Resolution_9bits);
	
  Radio.Rx(0);
	display_init();
	PRINTF("***********MOTHERSHIP RUNNING***********\n\r");
  while (1)
  {
		if(app_request)
		{
			app_request=0;
			/*
				read 1-wire sensor, save value and sent to display full set of temperatures
			*/
			
			static uint16_t cnt=0;
			indoor_temp = read_temp();
			PRINTF("Send to board: water: %i\t, indoor: %i\t, outdoor: %i\t, cnt: %d\n\r",water_temp,indoor_temp,outdoor_temp,cnt++);
		//display_set_temp(indoor_temp,outdoor_temp,water_temp);
		}
  }
}

void OnTxDone(void)
{
  Radio.Sleep();
  State = TX;
  PRINTF("OnTxDone\n\r");
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	TimerStart(&timerLed);
	LED_On(LED_BLUE);
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  State = RX;
  //if(strstr((char*)payload,"AC106_"))
	{
		if(rssi > - 100)LED_On(LED_RED1) ;
		if(rssi > - 80)LED_On(LED_RED2) ;
		if(rssi > - 60)LED_On(LED_BLUE) ;
		if(rssi > - 50)LED_On(LED_GREEN) ;
			
	}
	
	uint8_t decrypted_payload[APP_FRAME_LENGHT]={0};
	EncryptData(payload,APP_FRAME_LENGHT,decrypted_payload);

	if(decrypted_payload[0] == APP_HEADER)// nagłówek Ok
	{
		if(decrypted_payload[3] == CalculateCRC8(decrypted_payload, 3))// CRC Ok
		{
			switch(decrypted_payload[1])
			{
			case 0:water_temp = decrypted_payload[2]  ;PRINTF("Recieved Water Temp: %i\n\r",water_temp);break;
			case 1:outdoor_temp = decrypted_payload[2]  ;PRINTF("Recieved Outdoor Temp: %i\n\r",outdoor_temp);break;
			default: break;
			}
		}

	}
  //PRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
	Radio.Rx(0);
}

void OnTxTimeout(void)
{
  Radio.Sleep();
  State = TX_TIMEOUT;

  PRINTF("OnTxTimeout\n\r");
}

void OnRxTimeout(void)
{
  Radio.Sleep();
  State = RX_TIMEOUT;
  PRINTF("OnRxTimeout\n\r");
}

void OnRxError(void)
{
  Radio.Sleep();
  State = RX_ERROR;
  PRINTF("\n\rOnRxError\n\r");
	Radio.Rx(0);
}

static void OnledEvent(void *context)
{
  LED_Off(LED_BLUE) ;
  LED_Off(LED_RED1) ;
  LED_Off(LED_RED2) ;
  LED_Off(LED_GREEN) ;
}

static void OnTempEvent(void *context)
{
	TimerStart(&timerTemp);
	app_request=1;
}

static uint8_t CalculateCRC8(const uint8_t *data,int length)
{
	uint8_t crc = 0x00;
	uint8_t extract;
	uint8_t sum;
   for(uint8_t i=0;i<length;i++)
   {
      extract = *data;
      for (uint8_t tempI = 8; tempI; tempI--)
      {
         sum = (crc ^ extract) & 0x01;
         crc >>= 1;
         if (sum)
            crc ^= 0x8C;
         extract >>= 1;
      }
      data++;
   }
   return crc;
}

static void EncryptData(const uint8_t *input_data,int length, uint8_t *encrypted_data)
{
	uint8_t key[4] = { 0x67,0x89,0xab,0xcd};
	for(uint8_t i=0; i<length;i++)
	{
		encrypted_data[i] = input_data[i] ^ key[i];
	}

}

static int8_t read_temp()
{
	uint8_t  temperature_to_send;
	int8_t temperature_int8;

	DS18B20_ReadAll();
	DS18B20_StartAll();
	 
	 float temperature_float;
	 char message[64];

	 for(uint8_t i = 0; i < DS18B20_Quantity(); i++)
	 {
		if(DS18B20_GetTemperature(i, &temperature_float))
		{
			temperature_int8 = (int8_t)(round(temperature_float));// temperature_float w celach diagnostycznych
			temperature_to_send = abs(temperature_int8);

			if(temperature_int8 < 0) //sprawdzanie znaku
			{
				temperature_to_send |= 1<<7;
			}
			else
			{
					//do nothing;
			}	
		}
	 }
	 return temperature_to_send;
}
