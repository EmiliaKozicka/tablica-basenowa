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
#include "main.h"

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/* Led Timers objects*/
static  TimerEvent_t timerSend;

/* Private function prototypes -----------------------------------------------*/
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;


int main(void)
{

  HAL_Init();
  SystemClock_Config();
  DBG_Init();
  HW_Init();

  /*Disbale Stand-by mode*/
  LPM_SetOffMode(LPM_APPLI_Id, LPM_Disable);

  /* Send Timers*/
  TimerInit(&timerSend, OnSendEvent);
  TimerSetValue(&timerSend, SEND_INTERVAL_MS);
  TimerStart(&timerSend);

	RadioInit();
	
  

  while (1)
  {
		/*
		Radio.Rx(0);
		Master caly czas slucha 
		Slave idzie spac i czeka na wybudzenie timerem
		*/		
    DISABLE_IRQ();
    /* if an interupt has occured after __disable_irq, it is kept pending
     * and cortex will not enter low power anyway  */
    if (State == LOWPOWER)
    {
#ifndef LOW_POWER_DISABLE
      LPM_EnterLowPower();
#endif
    }
    ENABLE_IRQ();
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
	
  Radio.Sleep();
  BufferSize = size;
  memcpy(Buffer, payload, BufferSize);
  RssiValue = rssi;
  SnrValue = snr;
  State = RX;

  PRINTF("OnRxDone\n\r");
  PRINTF("RssiValue=%d dBm, SnrValue=%d\n\r", rssi, snr);
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
  PRINTF("OnRxError\n\r");
}

static void OnSendEvent(void *context)
{
  LED_Toggle(LED_BLUE) ;
  LED_Toggle(LED_RED1) ;
  LED_Toggle(LED_RED2) ;
  LED_Toggle(LED_GREEN) ;

  TimerStart(&timerSend);
}

void RadioInit(void)
{
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


}
