#ifndef __ESPPORTING_H
#define __ESPPORTING_H

#include "Arduino.h"
#include "byteswap.h"

#define os_time xTaskGetTickCount()/10
#define osMailQId QueueHandle_t
#define osTimerId TimerHandle_t
#define osTimerStart(t, d) xTimerStart(t, pdMS_TO_TICKS(d))
#define osTimerStop(t) xTimerStop(t, 0)
#define PT_YIELD(p) taskYIELD()

#ifdef FIRMWARE_VERSION_MAJOR
#undef FIRMWARE_VERSION_MAJOR
#endif
#define FIRMWARE_VERSION_MAJOR       0

#ifdef FIRMWARE_VERSION_MINOR   
#undef FIRMWARE_VERSION_MINOR
#endif
#define FIRMWARE_VERSION_MINOR       7

#define __REV(x)   __builtin_bswap32(x)
#define __REV16(x) __builtin_bswap16(x)

typedef struct {
    uint8_t reg;   // adress
    uint8_t value; // value
    uint8_t mask;  // mask
} SX1278_Config;

typedef struct {
    union {
        volatile uint32_t IRQ1;    
        volatile uint32_t IRQ1SET; 
        volatile uint32_t IRQ1CLR; 
    };
} LPC_MAILBOX_t;

#endif