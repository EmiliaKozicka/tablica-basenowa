#include "rc5.h"
#include "hw.h"
/*
    5 bit address and 6 bit rc5_command length (7 rc5_command bits for RC5X)
    Bi-phase coding (aka Manchester coding)
    Carrier frequency of 36kHz
    Constant bit time of 1.778ms (64 cycles of 36 kHz)
    Manufacturer Philips
*/
TIM_HandleTypeDef tim2;
uint8_t rc5_pulse_cnt=0;
uint8_t rc5_bit_cnt=0;
uint8_t rc5_bit_value=0;
uint8_t rc5_output=0;
uint8_t rc5_tx_ready=1;
uint16_t rc5_command=0;


void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&tim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
//nastepny puls
	rc5_pulse_cnt++;	
	if(rc5_command & 1<<(14-rc5_bit_cnt))
	{
		rc5_bit_value = 1;
	}
	else
	{
		rc5_bit_value = 0;
	}
		
	if(rc5_pulse_cnt == 64) 
	{
		rc5_pulse_cnt=0;//zerowanie
		rc5_bit_cnt++;//nastepny nit
		return;
	}	
		if(rc5_pulse_cnt < 32)
			{
				//pierwsZa polowa symbolu, wyjscie takie jak wartosc bitu
				if(rc5_bit_value)
				{
					LED_On(LED_BLUE);
					rc5_output=1;
				}
			}
			else
			{
				//druga polowa symbolu, wyjscie przeciwne do wartosci bitu
				if(!(rc5_bit_value))
				{
					LED_On(LED_BLUE);
					rc5_output=1;
				}
			}
	if(rc5_bit_cnt == 15)
	{
		rc5_bit_cnt=0;
		rc5_tx_ready=1;
		HAL_TIM_Base_Stop_IT(&tim2);
	}
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
				LED_Off(LED_BLUE);
			rc5_output=0;
		}	
}

void rc5_init()
{
	/*
	todo:
	inicjalizacja odpowiedniego pinu
	*/
		__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	
	tim2.Instance = TIM2;
	tim2.Init.Period = 880 - 1;
	tim2.Init.Prescaler = 1000 - 1;//1 - 1; <- ostatecznie ma byc tak
	tim2.Init.ClockDivision = 0;
	tim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
 
	__HAL_TIM_SET_COMPARE(&tim2, TIM_CHANNEL_1, 800);
	__HAL_TIM_ENABLE_IT(&tim2, TIM_IT_CC1);
	

}

void rc5_send_command(uint8_t command_to_send,uint8_t address)
{
	rc5_command = 3*4096 + address*64 + command_to_send; //zbicie daresu i komendy w zmienna do wyslania
	
	uint8_t rc5_pulse_cnt=0;
	uint8_t rc5_bit_cnt=1;
	uint8_t rc5_tx_ready=0;
	
	HAL_TIM_Base_Init(&tim2);
	HAL_TIM_Base_Start_IT(&tim2);
	
}
