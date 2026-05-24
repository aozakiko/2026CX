#ifndef _usart_H
#define _usart_H

#include "system.h"
#include "stdio.h"

void USART1_Init(u32 bound);
void USART1_SendByte(u8 data);
void USART1_SendBuffer(const u8 *buf, u16 len);
void USART1_SendString(const char *str);

#endif
