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
//#define osMailPut(q, p) (xQueueSend((q), (p), 0) == pdPASS ? osOK : osErrorOS)

#ifdef FIRMWARE_VERSION_MAJOR
#undef FIRMWARE_VERSION_MAJOR
#endif
#define FIRMWARE_VERSION_MAJOR       0

#ifdef FIRMWARE_VERSION_MINOR   
#undef FIRMWARE_VERSION_MINOR
#endif
#define FIRMWARE_VERSION_MINOR       51

#define __REV(x)   __builtin_bswap32(x)
#define __REV16(x) __builtin_bswap16(x)

typedef struct {
    uint8_t reg;   // Die Register-Adresse
    uint8_t value; // Der Wert für dieses Register
    uint8_t mask;  // Optional: Maske, falls nur einzelne Bits geändert werden sollen
} SX1278_Config;

// Definition des Mailbox-Typs
typedef struct {
    union {
        volatile uint32_t IRQ1;    // Der Name für den Lese-Zugriff
        volatile uint32_t IRQ1SET; // Der Name für den Schreib-Zugriff
        volatile uint32_t IRQ1CLR; // Der Name für den Schreib-Zugriff
    };
} LPC_MAILBOX_t;

#endif