/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "bsp/bsp.h"
#include "lora/utilities.h"

#include "../sx1276/include/sx1276/sx1276.h"

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 SX1276_NRESET

#define RADIO_NSS                                   MYNEWT_VAL(SPI_0_MASTER_SS_PIN)

#define RADIO_DIO_0                                 SX1276_DIO0
#define RADIO_DIO_1                                 SX1276_DIO1
#define RADIO_DIO_2                                 SX1276_DIO2
#define RADIO_DIO_3                                 SX1276_DIO3
#define RADIO_DIO_4                                 SX1276_DIO4
#define RADIO_DIO_5                                 SX1276_DIO5

#define RADIO_ANT_SWITCH_HF                         SX1276_ANT_HF_CTRL
//#define RADIO_ANT_SWITCH_LF                         1 //PA_1

//#define OSC_LSE_IN                                  1 //PC_14
//#define OSC_LSE_OUT                                 1 //PC_15

//#define OSC_HSE_IN                                  1 //PH_0
//#define OSC_HSE_OUT                                 1 //PH_1

//#define USB_DM                                      1 //PA_11
//#define USB_DP                                      1 //PA_12

//#define I2C_SCL                                     1 //PB_6
//#define I2C_SDA                                     1 //PB_7

//#define BOOT_1                                      1 //PB_2

//#define GPS_PPS                                     1 //PB_1
//#define UART_TX                                     1 //PA_9
//#define UART_RX                                     1 //PA_10

//#define DC_DC_EN                                    1 //PB_8
//#define BAT_LEVEL_PIN                               1 //PB_0
//#define BAT_LEVEL_CHANNEL                           1 //ADC_CHANNEL_8

//#define WKUP1                                       1 //PA_8
//#define USB_ON                                      1 //PA_2

#define RF_RXTX                                     SX1276_RXTX

//#define SWDIO                                       1 //PA_13
//#define SWCLK                                       1 //PA_14

//#define TEST_POINT1                                 1 //PB_12
//#define TEST_POINT2                                 1 //PB_13
//#define TEST_POINT3                                 1 //PB_14
//#define TEST_POINT4                                 1 //PB_15

//#define PIN_NC                                      1 //PB_5

/*!
 * Possible power sources
 */
enum BoardPowerSources
{
    USB_POWER = 0,
    BATTERY_POWER,
};

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

#endif // __BOARD_H__
