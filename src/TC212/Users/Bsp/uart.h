/*
 * uart.h
 *
 *  Created on: 2022-01-01
 *      Author: XinnZ & Pomin, Landian, HBUT
 *  XinnZ's Blog: https://blog.xinnz.cn/
 *  Pomin's Blog: https://www.pomin.top/
 */

#ifndef USERS_BSP_UART_H_
#define USERS_BSP_UART_H_

typedef struct
{
    void (*Init)(void);
    void (*Transmit)(const char *message, short *count);
    void (*Receive)(const char *message, short *count);

    volatile unsigned char FlagTransmitted;
    volatile unsigned char FlagReceived;
} UART_TypeDef;

extern UART_TypeDef UART_Console;
extern UART_TypeDef UART_EdgeBoard;

extern unsigned short CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT);

#endif /* USERS_BSP_UART_H_ */
