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
#include "hw.h"
#include "radio.h"
#include "timeServer.h"
#include "low_power_manager.h"
#include "vcom.h"
#include "rc5.h"
#if defined( REGION_AS923 )

#define RF_FREQUENCY                                923000000 // Hz

#elif defined( REGION_AU915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_CN470 )

#define RF_FREQUENCY                                470000000 // Hz

#elif defined( REGION_CN779 )

#define RF_FREQUENCY                                779000000 // Hz

#elif defined( REGION_EU433 )

#define RF_FREQUENCY                                433000000 // Hz

#elif defined( REGION_EU868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( REGION_KR920 )

#define RF_FREQUENCY                                920000000 // Hz

#elif defined( REGION_IN865 )

#define RF_FREQUENCY                                865000000 // Hz

#elif defined( REGION_US915 )

#define RF_FREQUENCY                                915000000 // Hz

#elif defined( REGION_RU864 )

#define RF_FREQUENCY                                864000000 // Hz

#else
#error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
//  1: 250 kHz,
//  2: 500 kHz,
//  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
//  2: 4/6,
//  3: 4/7,
//  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false



typedef enum
{
  LOWPOWER,
  RX,
  RX_TIMEOUT,
  RX_ERROR,
  TX,
  TX_TIMEOUT,
} States_t;

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 64 // Define the payload size here
#define LED_PERIOD_MS               200

/* APP DEFINES*/
#define APP_HEADER 			0xb5
#define APP_FRAME_LENGHT 	5
#define APP_KEY				{0x45, 0x67,0x89,0xab,0xcd,0xef}


#define LEDS_OFF   do{ \
                   LED_Off( LED_BLUE ) ;   \
                   LED_Off( LED_RED ) ;    \
                   LED_Off( LED_GREEN1 ) ; \
                   LED_Off( LED_GREEN2 ) ; \
                   } while(0) ;


uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  TimerEvent_t timerLed;

/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone(void);

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout(void);

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout(void);

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError(void);

/*!
 * \brief Function executed on when led timer elapses
 */
static void OnledEvent(void *context);

/*!
 * \brief Function calculate crc8
 */
static uint8_t CalculateCRC8(const uint8_t *data,int length);

/*!
 * \brief Function encrypt input data
 */
static void EncryptData(const uint8_t *input_data,int length, uint8_t *encrypted_data);

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


  //Radio.Rx(0);
	rc5_init();
	PRINTF("***********MOTHERSHIP RUNNING***********\n\r");
	int8_t temp = 0;
  while (1)
  {
//		LED_On(LED_GREEN)
//		DelayMs(100);
//		LED_Off(LED_GREEN)
//		BufferSize = 4;
//		Buffer[0] = 0xb5;//nagłówek
//		Buffer[1] = temp%2;//adres
//		Buffer[2] = temp++;//temp 16*C
//		Buffer[3] = CalculateCRC8(Buffer,3);
//		
//		uint8_t EncryptedBuffer[4] = {0};
//		
//		EncryptData(Buffer,4,EncryptedBuffer);
//		
//		Radio.Send(EncryptedBuffer, BufferSize);
		DelayMs(400);
		rc5_send_command(0x35,0x05);
		DelayMs(40100);
		
 
  
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
			case 0:PRINTF("WaterTemp: %i\n\r",decrypted_payload[2]);/*DisplayWaterTemp(decrypted_payload[2])*/ ;break;
			case 1:PRINTF("OutdoorTemp: %i\n\r",decrypted_payload[2]);/*DisplayOutdoorTemp(decrypted_payload[2])*/  ;break;
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

