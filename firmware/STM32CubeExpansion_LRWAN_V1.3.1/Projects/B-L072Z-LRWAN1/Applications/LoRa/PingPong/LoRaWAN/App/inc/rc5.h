#include <stdint.h>

#define RC5_MSG_LENGHT 14

void rc5_init(void);
void rc5_send_command(uint8_t command_to_send,uint8_t address);
