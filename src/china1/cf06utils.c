
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "lpclib.h"
#include "cf06.h"
#include "cf06private.h"
#include "reedsolomon.h"

#include "bridge.h"

/* Check CRC of a outer block */
_Bool _CF06_checkCRCOuter (uint8_t *buffer, int length, uint16_t receivedCRC)
{
#ifdef ARDUINO_ARCH_ESP32
    return (receivedCRC == getCRC2(buffer, length, 0x0000));
#else
    CRC_Handle crc = LPCLIB_INVALID_HANDLE;
    CRC_Mode crcMode;
    bool result = false;

    crcMode = CRC_makeMode(
            CRC_POLY_CRCCCITT,
            CRC_DATAORDER_NORMAL,
            CRC_SUMORDER_NORMAL,
            CRC_DATAPOLARITY_NORMAL,
            CRC_SUMPOLARITY_NORMAL
            );
    if (CRC_open(crcMode, &crc) == LPCLIB_SUCCESS) {
        CRC_seed(crc, 0x0000);
        CRC_write(crc, buffer, length, NULL, NULL);

        result = receivedCRC == CRC_read(crc);

        CRC_close(&crc);
    }

    return result;
#endif
}


/* Check CRC of inner block */
_Bool _CF06_checkCRCInner (uint8_t *buffer, int length, uint16_t receivedCRC)
{
#ifdef ARDUINO_ARCH_ESP32
    return (receivedCRC == getCRC(buffer, length));
#else
    CRC_Handle crc = LPCLIB_INVALID_HANDLE;
    CRC_Mode crcMode;
    bool result = false;

    crcMode = CRC_makeMode(
            CRC_POLY_CRCCCITT,
            CRC_DATAORDER_NORMAL,
            CRC_SUMORDER_NORMAL,
            CRC_DATAPOLARITY_NORMAL,
            CRC_SUMPOLARITY_NORMAL
            );
    if (CRC_open(crcMode, &crc) == LPCLIB_SUCCESS) {
        CRC_seed(crc, 0x0000);
        CRC_write(crc, buffer, length, NULL, NULL);

        result = receivedCRC == CRC_read(crc);

        CRC_close(&crc);
    }

    return result;
#endif
}


/* Struktur zur Kapselung der RS-Daten */
typedef struct {
    uint8_t *rawFrame;
    uint8_t nullByte;
} CF06_RS_Context;

/* Statische Instanz für den Callback-Zugriff */
static CF06_RS_Context _outerContext;

/* Die eigentliche Callback-Funktion (jetzt eine reguläre Funktion) */
static uint8_t* _CF06_getDataAddressOuter(int index)
{
    if (index < 89) {
        return &_outerContext.rawFrame[93 - index];
    }
    else if (index >= 249) {
        return &_outerContext.rawFrame[348 - index];
    }
    else {
        _outerContext.nullByte = 0;
        return &_outerContext.nullByte;
    }
}

/* Reed-Solomon error correction */
LPCLIB_Result _CF06_checkReedSolomonOuter (uint8_t rawFrame[], int *pNumErrors)
{
    int numErrorsOuter = 0;
    LPCLIB_Result result;
    uint8_t _CF06_null;



    // uint8_t * _CF06_getDataAddressOuter (int index)
    // {
    //     if (index < 89) {
    //         return &rawFrame[93 - index];
    //     }
    //     else if (index >= 249) {
    //         return &rawFrame[348 - index];
    //     }
    //     else {
    //         _CF06_null = 0;
    //         return &_CF06_null;
    //     }
    // }

    _outerContext.rawFrame = rawFrame;
    _outerContext.nullByte = 0;

    /* Make sure the correct Galois field is used */
    REEDSOLOMON_makeGaloisField(0x11D);

    /* Reed-Solomon error correction for outer frame */
    result = REEDSOLOMON_process(6, 1, 1, 1, _CF06_getDataAddressOuter, &numErrorsOuter);

    if (pNumErrors) {
        *pNumErrors = numErrorsOuter;
    }

    return result;
}


/* Globale oder statische Hilfsvariablen (nicht thread-sicher!) */
static uint8_t *g_currentRawFrame;
static uint8_t g_rs_null_byte;

/* Reguläre Funktion außerhalb von _CF06_checkReedSolomonInner */
static uint8_t* _CF06_getDataAddressInner(int index)
{
    if (index < 42) {
        return &g_currentRawFrame[46 - index];
    }
    else if (index >= 249) {
        return &g_currentRawFrame[301 - index];
    }
    else {
        g_rs_null_byte = 0;
        return &g_rs_null_byte;
    }
}

/* Reed-Solomon error correction */
LPCLIB_Result _CF06_checkReedSolomonInner (uint8_t rawFrame[], int *pNumErrors)
{
    int numErrorsInner = 0;
    LPCLIB_Result result;
    uint8_t _CF06_null;

    g_currentRawFrame = rawFrame;

    // uint8_t * _CF06_getDataAddressInner (int index)
    // {
    //     if (index < 42) {
    //         return &rawFrame[46 - index];
    //     }
    //     else if (index >= 249) {
    //         return &rawFrame[301 - index];
    //     }
    //     else {
    //         _CF06_null = 0;
    //         return &_CF06_null;
    //     }
    // }


    /* Reed-Solomon error correction for inner frame */
    result = REEDSOLOMON_process(6, 1, 1, 1, _CF06_getDataAddressInner, &numErrorsInner);

    if (pNumErrors) {
        *pNumErrors = numErrorsInner;
    }

    return result;
}

