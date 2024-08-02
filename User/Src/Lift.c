
#include "stdint.h"
#include "Lift.h"
#include "usart.h"


/**
 *
 * @param command 命令可为0 1 2 ，0为上升，1为停止，2为下降
 * @return
 */
uint8_t Stretcher_rises_control (uint8_t command) {
    uint8_t Stretcher_command[3][8] = {
        {0x01,0x06,0x00,0x01,0x00,0x01,0x19,0xCA},
        {0x01,0x06,0x00,0x02,0x00,0x01,0xE9,0xCA},
        {0x01,0x06,0x00,0x03,0x00,0x01,0xB8,0x0A}
    };
    if (HAL_UART_Transmit(&STRETCHER_UART_HALDEL,Stretcher_command[command],8,1000) == HAL_OK) {
        return 1;
    }else {
        return 0;
    }
}

