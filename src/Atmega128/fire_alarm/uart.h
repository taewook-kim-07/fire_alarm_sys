/*
 * uart.h
 *
 * Created: 2021-06-03 목 오후 7:30:35
 */ 

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>

void UART_initialize(void);						// UART 초기화

void UART_transmit(unsigned char data);			// 1 바이트송신
void UART_printString(char *str);				// 문자열송신

#endif