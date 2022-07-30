/*
 * def.h
 *
 *  Created on: Jul 29, 2022
 *      Author: wvv
 */

#ifndef SRC_DEF_H_
#define SRC_DEF_H_

#include <stdio.h>

#define HIGH 1
#define LOW 0
#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL ((void *)0)
#endif

typedef void (*pFunUart)(uint8_t*, uint32_t);
typedef void (*pFunVoid)();
typedef void (*pFunPtr)(void *p);

extern volatile uint32_t uwTick;
#define LOG_LINE(lvl, fmt, ...)                                   \
    do{                                                           \
        printf("[%10lu][" lvl "]" fmt "\r\n", uwTick, ##__VA_ARGS__); \
    } while (0)

#define log_e(fmt, ...) LOG_LINE("E", fmt, ##__VA_ARGS__)
#define log_d(fmt, ...) LOG_LINE("D", fmt, ##__VA_ARGS__)
#define log_i(fmt, ...) LOG_LINE("I", fmt, ##__VA_ARGS__)
#define log_hex(header, data, len) 						\
		do{												\
		printf("[%10lu][HEX][]" header "]\r\n", uwTick);	\
		for(uint8_t _i = 0;_i<len;_i++)printf("%02X ",data[_i]);\
		printf("\r\n");\
		}while(0)

#endif /* SRC_DEF_H_ */
