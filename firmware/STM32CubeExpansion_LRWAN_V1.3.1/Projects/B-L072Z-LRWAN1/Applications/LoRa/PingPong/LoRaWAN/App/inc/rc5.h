#include <stdint.h>

#define RC5_IR_OUTPUT_PIN GPIO_PIN_0
#define RC5_IR_OUTPUT_PORT GPIOA

#define RC5_MSG_LENGHT 14

void rc5_init(void);
void rc5_send_command(uint8_t command_to_send,uint8_t address);
void rc5_ir_output_on();
void rc5_ir_output_off();