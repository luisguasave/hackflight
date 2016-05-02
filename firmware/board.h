#pragma once

#include <stdint.h>
#include <stdbool.h>

// Your implementation should define the following

#include <printf.h>

void     board_delayMilliseconds(uint32_t msec);
uint32_t board_getMicros();
void     board_imuInit(uint16_t * acc1G, float * gyroScale);
void     board_imuReadAccel(int16_t * data);
void     board_imuReadGyro(int16_t * data);
void     board_init(void);
void     board_ledOff(void);
void     board_ledOn(void);
uint16_t board_pwmRead(uint8_t chan);
void     board_writeMotor(uint8_t index, uint16_t value);
uint8_t  board_serialAvailable(void);
uint8_t  board_serialRead(void);
void     board_serialWrite(uint8_t c);

bool     board_baroInit(void);
int32_t  board_baroReadPressure(void);
bool     board_sonarInit(void);
int32_t  board_sonarReadDistance(void);

void     board_checkReboot(bool pendReboot);
uint16_t board_getI2cErrorCounter(void);
void     board_reboot(void);