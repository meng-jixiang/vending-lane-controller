/**
 * @file    laser.h
 * @brief   Device 层 — 光幕控制
 */
#ifndef __LASER_H
#define __LASER_H

#include <stdint.h>

void    Laser_Init(void);
uint8_t Laser_SelfCheck(void);        /* 自检: 0=通过, 1=有遮挡 */
uint8_t Laser1_IsBlocked(void);       /* 1=有遮挡 */
uint8_t Laser2_IsBlocked(void);       /* 1=有遮挡 */
void    Laser_StartDetect(void);      /* 开始掉货检测 */
uint16_t Laser_GetBlockTime_ms(void); /* 遮挡累计时间 */

#endif
