/*
 * main.c
 *
 * Created: 2021-06-03 목 오후 7:12:49
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ATmega128_v20.h"
#include "uart.h"

int ad_conversion(char);
int map(int, int, int, int ,int);	// 입력한 데이터를 범위를 지정하여 비례화 함

/* 통신 관련 */
#define BUFFER_SIZE		16	// 송신 버퍼 크기
#define SEND_RATE_MS	500	// 송신 주기 (Polling이 아닌 Event 방식이나 연속적인 값 변화로 지속적인 SERIAL 통신을 막기 위함)
#define SEND_TEMP		"temp|"				// 송신, 온도 접두사
#define SEND_CO2		"co2|"				// 송신, co2 접두사
#define SEND_ALRAM		"alarm|"			// 송신, 알람상태 접두사
#define SEND_END		20					// 송신의 끝을 알리는 문자

#define RECV_ALARM		"salarm"	// 수신, 알람 관련 명령어 (0, 2)
#define RECV_MAXTEMP	"mtemp"		// 수신, 감지 온도 명령어
#define RECV_MAXCO2		"mco2"		// 수신, 감지 CO2 명령어

int send_wait = 0;	// SERIAL 송신 상태 (0: 전송 가능, 1: 전송 대기)
const unsigned int send_rate = SEND_RATE_MS/10;	// 상수형, SEND_RATE_MS/10	(TIMER0은 10ms로 동작되어서)

/* 부저 관련 */
#define BUZZER_RATE		300	// Buzzer의 STEP 변경 주기
#define MAX_TONE		9	// tone_table의 Array Size
#define REAL_SIT_STEP	2	// 실제 경보일 때 Buzzer의 음은 2개로 반복
#define DRILL_SIT_STEP	5	// 훈련 경보일 때 Buzzer의 음은 3개로 반복

enum { NONE = 0, DO, RE, MI, PA, SOL, RA, SI, DDO };	// tone_table을 배열의 위치를 선택할 때 숫자보다 문자가 편하여 열거형 사용 { 0, 1, 2, ... }
const char tone_table[MAX_TONE] = {1, 17, 43, 66, 77, 97, 114, 117, 137};	// 무음 도 레 미 파 솔 라 시 도의 분주비
volatile unsigned char buzzer_tone = 0;		// 현재 Buzzer 음계
int buzzer_timer = 0;	// Buzzer의 음계 변경 주기를 확인하기 위한 카운터
unsigned char buzzer_step = 0;	// 출력 시킬 Buzzer의 음계 step
const unsigned int buzzer_rate = BUZZER_RATE/10;	// 상수형으로 BUZZER_RATE를 10으로 나눔 (if(비교==BUZZER_RATE/10)의 경우일 경우 불필요하게 같은 나눗셈을 하기 때문)

/* 램프 관련 */
#define LAMP_RATE		200	// 램프 점멸 간격
int lamp_timr = 0;			// 램프의 점멸 주기 카운터
const unsigned int lamp_rate = LAMP_RATE/10;	// 상수형으로 BUZZER_RATE를 10으로 나눔 (if(비교==BUZZER_RATE/10)의 경우일 경우 불필요하게 같은 나눗셈을 하기 때문)

/* 기타 */
char alarm = 0;		// 현재 알람 상태 (0: Off, 1: Real, 2: Drill)
int temp = 0, co2 = 0;	// 현재 온도, 현재 co2 농도 변수
int max_temp = 100, max_co2 = 2000;	// 최대 온도, 최대 co2 농도 변수 조건

ISR(TIMER0_OVF_vect)
{
	//10ms 수행
	TCNT0 = 100;	//10ms 인터럽트 재 초기화
	
	static int old_temp = 0, old_co2 = 0;	// 이전의 온도와 co2 농도
	static char old_alarm = 0;				// 이전의 알람 상태
	char type = 0;	// 변화가 있는 데이터 확인 (0: 변화 없음, 1: 온도 변화, 2: CO2 변화, 4: 경보 변화, 7: 모두 변화)

	if(alarm)	// 알람이 켜진 경우 (1: 실제 경보, 2: 훈련 경보)
	{
		// 아래는 램프 관련
		if(++lamp_timr >= lamp_rate)
		{
			lamp_timr = 0;
			if(alarm == 1)
				PORTB = (PORTB + 0x20) & 0x2F;	// 상위 비트 2번째에 2를 더하고 상위 2비트만 가져온다 (0 또는 1만 반복)
			else
				PORTB = (PORTB + 0x10) & 0x1F;	// 상위 비트 1번째에 1를 더하고 상위 2비트만 가져온다 (0 또는 1만 반복)
		}
		
		// 아래는 부저 관련
		if(++buzzer_timer >= buzzer_rate)	// Buzzer타이머가 Buzzer카운터보다 높을 경우
		{
			buzzer_timer = 0;
			if(alarm == 1 && ++buzzer_step >= REAL_SIT_STEP)	// 실제 경보일 경우, 음계 변경
			{
				buzzer_step = 0;
			}else if(alarm == 2 && ++buzzer_step >= DRILL_SIT_STEP)	// 훈련 경보일 경우, 음계 변경
			{
				buzzer_step = 0;
			}
		}
		
		if(alarm == 1)			// 실제 경보일 경우 설정된 음계, 시-솔-
		{
			if(buzzer_step)
				buzzer_tone = SI;
			else
				buzzer_tone = SOL;
		}else if(alarm == 2)	// 훈련 경보일 경우 설정된 음계, 도-레-미-
		{
			switch(buzzer_step)
			{
				case 0: buzzer_tone = DO; break;
				case 1: buzzer_tone = RE; break;
				case 2: buzzer_tone = MI; break;
				case 3: buzzer_tone = PA; break;
				default: buzzer_tone = SOL; break;		
			}			
		}		
	} else {	// 알람이 꺼진 경우, Buzzer도 끄고 Buzzer관련 변수 초기화
		buzzer_tone = NONE;
		buzzer_timer = 0;
		buzzer_step = 0;
		
		if(PORTB & 0xE0 || (~PORTB) & 0x10)	// LAMP1외 에 켜지거나 LAMP1이 켜지지 않은 경우
		{
			PORTB &= 0x0F;	// 모든 램프 소등
			PORTB |= 0x10;	// 램프 1번 점등
		}
	}
	
	if(send_wait-- <= 0)	// 송신 가능할 경우
	{		
		if(temp != old_temp)	// 온도가 현재 값과 이전 값 다른 경우
		{
			old_temp = temp;	// 이전 온도에 현재 온도 대입
			type += 1;			// type 값 0001을 더함
		}
		if(co2 != old_co2)		// co2 농도가 현재 값과 이전 값 다른 경우
		{		
			old_co2 = co2;		// 이전 co2 농도에 현재 co2 농도 대입
			type += 2;			// type 값 0010을 더함
		}
		if(alarm != old_alarm)	// 경보 알람이 현재 값과 이전 값 다른 경우
		{
			old_alarm = alarm;	// 이전 경보 값에 현재 경보 값을 대입
			type += 4;			// type 값 0100을 더함
		}// 3개의 값(온도,CO2, 경보)이 모두 변하면 type이 최대 0111이 됨

		if(type)	// TEMP, CO2, ALARM 중 변화가 있으면 SERIAL 전송
		{
			char data[BUFFER_SIZE] = "";	// 전송할 빈 문자열 생성
			if(type & 0x01)	// 온도값이 바뀐 경우
			{
				sprintf(data, "%s%d%c", SEND_TEMP, old_temp, SEND_END);	// temp|온도값 문자열 대입
				UART_printString(data);						// Serial 전송
			}
			if(type & 0x02)	// co2 농도값이 바뀐 경우
			{
				sprintf(data, "%s%d%c", SEND_CO2, old_co2, SEND_END);	// co2|농도값 문자열 대입
				UART_printString(data);						// Serial 전송
			}
			if(type & 0x04)	// 경보 알람이 바뀐 경우
			{
				sprintf(data, "%s%d%c", SEND_ALRAM, old_alarm, SEND_END);	// alarm|현재상태 문자열 대입
				UART_printString(data);							// Serial 전송
			}
			type = 0; // type 초기화
			send_wait = send_rate;	// 50을 대입하여 다음 송신까지 대기가 됨	
		}		
	}
}

ISR(TIMER2_OVF_vect)	// 32분주비로 동작하며, Buzzer의 음계에 따라 주기가 변경됨
{
	static unsigned char state = 0;	// Buzzer의 ON/OFF를 조절하여 제어
	if(state == 0 && buzzer_tone > 0) {		// Buzzer 음계가 None이 아닌 경우 Buzzer를 울릴 수 있음
		PORTG |= _BV(3);	// Buzzer on
		state = 1;			// Buzzer 상태 1
	} else {
		PORTG &= ~_BV(3);	// buzzer off
		state = 0;			// Buzzer 상태 0
	}
	TCNT2 = tone_table[buzzer_tone];	// 만약, None일 경우 0.51 msec 후 동작
}

ISR(USART0_RX_vect)		// UART 인터럽트 수신
{
	static char rx_buffer[BUFFER_SIZE] = "";	// 수신 데이터의 버퍼를 생성
	static unsigned char rx_size = 0;			// 수신 데이터의 크기 계산
	
	if(rx_size == BUFFER_SIZE) { memset(rx_buffer, 0, BUFFER_SIZE); rx_size = 0; }	// 버퍼를 초과할 경우 데이터를 초기화함
		
	rx_buffer[rx_size++] = UDR0;	// 수신된 Serial 통신의 값
	if(rx_buffer[rx_size] == '\0')	// 수신된 데이터가 문자열의 null인 경우
	{
		char* split= strtok(rx_buffer, "|");	// 명령어 접두사
		if (split != NULL)
		{
			if(strcmp(split, RECV_ALARM) == 0)	// 명령어 코드
			{
				split = strtok(NULL, "|");
				if (split != NULL)
				{
					char data = atoi(split);
					switch(data)
					{
						case 0:
						case 1:
						case 2: alarm = data; break;
						default: break;						
					}
				}
			} else if(strcmp(split, RECV_MAXTEMP) == 0) {
				split = strtok(NULL, "|");
				if (split != NULL)
				{
					max_temp = atoi(split);
				}
			} else if(strcmp(split, RECV_MAXCO2) == 0) {
				split = strtok(NULL, "|");
				if (split != NULL)
				{
					max_co2 = atoi(split);
				}
			}			
		}
		/*if(strcmp(rx_buffer, RECV_START_ALARM) == 0) {	// 수신 데이터와 알람 작동 명령어 대조
			alarm = 2;
		} else if(strcmp(rx_buffer, RECV_STOP_ALARM) == 0) {	// 수신 데이터와 알람 정지 명령어 대조
			alarm = 0;
		}*/
		memset(rx_buffer, 0, BUFFER_SIZE);	// 수신 데이터 초기화
		rx_size = 0;						// 수신 데이터의 크기 계산 초기화
	}
}

int main(void)
{
	MCU_initialize();	//DDRx 선언
	Delay_ms(50);
	UART_initialize();

	TCCR0=0x00;	//기능 정지	
	TCNT0=100;	//10ms ISR
	TCCR0 |= (7<<CS00);	// TCCR0|=0x07;	분주비 1:1024
	
	TCCR2=0x00;
	TCNT0=0;
	TCCR2 |= (1<<CS01) | (1<<CS00);	// TCCR0|=0x03;	분주비 1:32

	TIMSK |= (1<<TOIE0) | (1<<TOIE2);	//TIMSK|=0x01;	개별인터럽트 활성화	
	sei();	//SREG|=0x80;	글로벌 인터럽트 활성화

	DDRB &= 0x0F;	// DDRB
	
	char key = 0;
    while (1) 
    {
	    temp = map(ad_conversion(0), 0, 1023, 0, 100);	// 0-1023을 0-100로 변환
	    co2  = map(ad_conversion(1), 0, 1023, 0, 2000);	// 0-1023을 0-2000로 변환
		if(temp >= max_temp || co2 >= max_co2)			// 현재 온도가 한계 온도를 넘거나 co2 농도가 한계 농도를 넘거나
		{
			alarm = 1;	// 실제 경보 알람 작동
		}
		
		key=KEY_INPUT();
		switch(key)
		{
			case 0x0e:
				alarm = 1;				
				break;
			case 0x0d:
				alarm = 0;
				break;
			default :
				break;
		}
    }
}

int ad_conversion(char ch)	//가변저항 설정
{
	ADMUX = (ch&0x1f) | 0x40;
	ADCSRA = 0xD5;
	while((ADCSRA&0x10) != 0x10);
	return(ADC);
}

int map(int x, int in_min, int in_max, int out_min, int out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int sringtonum(char* text)	// atoi 대체
{
	int result = 0, i = 0;
	while(text[i] != '\0')
	{
		if(text[i] > '0' || text[i] < '9')
		{
			result += text[i] - '0';
		}else{
			result = 0;
			break;
		}
		i++;
	}	
	return result;
}