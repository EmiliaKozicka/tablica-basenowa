#include "display.h"
#include "hw.h"
#include "rc5.h"
/*
-----------------------------------------------
|					 		  17:41													|
|							temperatura											|
|wody 27			powietrza 29		na zewnatrz -9	|
-----------------------------------------------
Pilot od tablicy na basenie:
F+ time
F- temp
menu zatwierdzenie
warotsci ch+ / ch-
pozycje vol+/vol-
*/


void display_init(void)
{
	rc5_init();
}

void display_set_time(uint8_t hours,uint8_t minutes)
{
	rc5_send_command(DISPLAY_CHANGE_TIME,0);
	
	rc5_send_command(hours/10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(hours%10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(minutes/10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(minutes%10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(DISPLAY_SAVE,0);
}

void display_set_temp(int8_t indoor_temp,int8_t outdoor_temp,int8_t water_temp)
{
	rc5_send_command(DISPLAY_CHANGE_TEMP,0);
			//water
	rc5_send_command(water_temp/10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(water_temp%10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	//indoor
	rc5_send_command(indoor_temp/10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(indoor_temp%10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
		//outdoor
	if(outdoor_temp<0) rc5_send_command(DISPLAY_OUTDOOR_MINUS,0);
		
	rc5_send_command(outdoor_temp/10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(outdoor_temp%10,0);
	rc5_send_command(DISPLAY_NEXT_DIGIT,0);
	
	rc5_send_command(DISPLAY_SAVE,0);

}

void display_test(void)
{
		rc5_send_command(1,0);
		rc5_send_command(2,0);
		rc5_send_command(3,0);
		rc5_send_command(4,0);
		rc5_send_command(5,0);
		rc5_send_command(6,0);
		rc5_send_command(7,0);
		rc5_send_command(8,0);
		rc5_send_command(9,0);

}
