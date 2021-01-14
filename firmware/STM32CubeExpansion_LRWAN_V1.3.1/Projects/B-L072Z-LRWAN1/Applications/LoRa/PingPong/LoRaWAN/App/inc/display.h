#include <stdint.h>
/*
					 17:41
				temperatura
wody 27		powietrza 29		na zewnatrz -9

*/

#define PILOT_0					0x00
#define PILOT_1					0x01
#define PILOT_2					0x02
#define PILOT_3					0x03
#define PILOT_4					0x04
#define PILOT_5					0x05
#define PILOT_6					0x06
#define PILOT_7					0x07
#define PILOT_8					0x08
#define PILOT_9					0x09


#define PILOT_F_PLUS		0X2B //uruchomienie zmiany godziny
#define DISPLAY_CHANGE_TIME PILOT_F_PLUS
#define PILOT_F_MINUS 	0X2C //uruchomienie zmiany temperatury
#define DISPLAY_CHANGE_TEMP PILOT_F_MINUS

#define	PILOT_MINUS			0X11 //przejscie do zmiany kolejnej pozycji temperatur
#define DISPLAY_PREV_DIGIT PILOT_MINUS
#define	PILOT_PLUS			0X10 //przejscie do zmiany poprzedniej pozycji temperatur
#define DISPLAY_NEXT_DIGIT PILOT_PLUS			

#define PILOT_CH_PLUS		0X20 //zwiekszenie aktualnie modyfikowanej cyfry
#define DISPLAY_INCREMENT PILOT_CH_PLUS
#define PILOT_CH_MINUS	0X21 //zmniejszenie aktualnie modyfikowanej cyfry 
#define DISPLAY_DECREMENT PILOT_CH_MINUS

#define PILOT_BLUE			0X34 //'-' przy temperaturze zewnetrznej
#define DISPLAY_OUTDOOR_MINUS PILOT_BLUE

#define PILOT_MENU			0X3B //zatwierdzenie zmian
#define DISPLAY_SAVE PILOT_MENU

void display_init(void);
void display_set_time(uint8_t hours,uint8_t minutes);
void display_set_temp(int8_t indoor_temp,int8_t outdoor_temp,int8_t water_temp);
void display_test(void);
