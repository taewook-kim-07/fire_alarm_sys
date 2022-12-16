/*
 * uart.c
 *
 * Created: 2021-06-03 목 오후 7:32:55
 */ 

#include "uart.h"

#define BAUDRATE	9600	// 통신 속도
#define X2MODE				// 2배속 (주석 시 1배속)

// UBRB0 계산
#ifdef X2MODE
	#define UBRR0_VAL	((F_CPU/(8UL*BAUDRATE))-1)	// f/(8*BAUDRATE)-1
#else
	#define UBRR0_VAL	((F_CPU/(16UL*BAUDRATE))-1)	// f/(16*BAUDRATE)-1
#endif

void UART_initialize(void) // UART초기화
{
	#ifdef X2MODE
		UCSR0A |= (1<<U2X0);	// 2배속 모드
	#else
		UCSR0A |= (0<<U2X0);	// 1배속 모드
	#endif
	
	// 수신 인터럽트활성, 수신활성, 송신활성
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	// 0: 비동기모드, 00: No Parity, 0: Stop bit 1bit, 11: 8 bit, 0: 송신상승, 수신하강 엣지
	UCSR0C = (0<<UMSEL0)|(0<<UPM00)|(0<<USBS0)|(0b11<<UCSZ00)|(0<<UCPOL0);

	UBRR0H = (uint8_t)(UBRR0_VAL>>8);	// UBRR0의 상위바이트
	UBRR0L = (uint8_t)(UBRR0_VAL);		// UBRR0의 하위바이트	
}

void UART_transmit(unsigned char data)
{
	while(!(UCSR0A & (1<<UDRE0)));	// UCSR0A 5번 비트에서 0이면 송신될 버퍼의 데이터 없음
	UDR0=data;	// 송신 버퍼에 데이터 저장
}

void UART_printString(char *str)
{
	int i = 0;
	while(str[i] != '\0') UART_transmit(str[i]);	// 문자열의 끝이 될때가지 전송
}