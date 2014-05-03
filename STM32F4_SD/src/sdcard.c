/**
 * @file: 	sdcard.c
 * @brief:	   
 * @date: 	22 kwi 2014
 * @author: Michal Ksiezopolski
 * 
 * @verbatim
 * Copyright (c) 2014 Michal Ksiezopolski.
 * All rights reserved. This program and the 
 * accompanying materials are made available 
 * under the terms of the GNU Public License 
 * v3.0 which accompanies this distribution, 
 * and is available at 
 * http://www.gnu.org/licenses/gpl.html
 * @endverbatim
 */

#include "sdcard.h"
#include "spi.h"
#include "timers.h"
#include <stdio.h>


/*
 * SD commands as per SanDisk Secure Digital Card product manual.
 */
#define SD_GO_IDLE_STATE 		0 	///< Resets SD Card.
#define SD_SEND_OP_COND 		1 	///< Activates the card initialization process.
#define SD_SEND_CSD 			9 	///< Ask for card specific data (CSD).
#define SD_SEND_CID				10	///< Ask for card identification (CID).
#define SD_STOP_TRANSMISSION	12	///< Forces a card to stop transmission during a multiple block read operation.
#define SD_SEND_STATUS			13	///< Ask for status register contents.
#define SD_SET_BLOCKLEN			16	///< Selects block length in bytes for all following block commands
#define SD_READ_SINGLE_BLOCK	17	///< Reads a block of size set by SET_BLOCKLEN
#define SD_READ_MULTIPLE_BLOCK 	18	///< Continuously transfers data blocks from card to host until interrupted by STOP_TRANSMISSION
#define SD_WRITE_BLOCK			24	///< Writes a block of size set by SET_BLOCKLEN
#define SD_WRITE_MULTIPLE 		25	///< Continuously writes blocks of data until a stop transmission token is sent


static uint8_t SD_SendCommand(uint8_t cmd, uint8_t* args, uint8_t crc);

/**
 * @brief Initialize the SD card.
 */
void SD_Init(void) {

	SPI1_Init(); // Initialize SPI interface.

	int i;

	for (i = 0; i < 10; i++) {
		SPI1_Transmit(0xff);
	}

	uint8_t status;
	uint8_t args[4] = {0, 0, 0, 0};

	i = 0;
	do {
		status = SD_SendCommand(0x40, args, 0x95);
		TIMER_Delay(5);
		i++;
	} while (status != 1);

	printf("CMD0, status = %02x (%d iterations)\r\n", status, i);

	args[2] = 0x01;
	args[3] = 0xaa;

	i = 0;
	do {
		status = SD_SendCommand(0x48, args, 0x87);
		TIMER_Delay(5);
		i++;
	} while (status != 1);

	printf("CMD8, status = %02x (%d iterations)\r\n", status, i);

	args[2] = 0;
	args[3] = 0;

	i = 0;
	do {
		args[0] = 0;
		status = SD_SendCommand(0x77, args, 0xff);
		args[0] = 0x40;
		TIMER_Delay(5);
		status = SD_SendCommand(0x69, args, 0xff);
		i++;
		TIMER_Delay(50);
	} while (status != 0);

	printf("ACMD41, status = %02x (%d iterations)\r\n", status, i);

	args[0] = 0;
	i = 0;
	do {
		status = SD_SendCommand(0x40+58, args, 0xff);
		i++;
		TIMER_Delay(50);
	} while (status != 0);

	printf("OCR, status = %02x (%d iterations)\r\n", status, i);

	// 4 bytes of OCR
	SPI1_Select();
	SPI1_Transmit(0xff);
	SPI1_Transmit(0xff);
	status = SPI1_Transmit(0xff);
	SPI1_Transmit(0xff);
	SPI1_Transmit(0xff);
	SPI1_Transmit(0xff); // One additional for safety

	printf("OCR, 3rd byte  = %02x\r\n", status);

	SPI1_Deselect();
}

static uint8_t SD_SendCommand(uint8_t cmd, uint8_t* args, uint8_t crc) {

	SPI1_Select();
	SPI1_Transmit(0xff);

	SPI1_Transmit(cmd);
	SPI1_Transmit(args[0]);
	SPI1_Transmit(args[1]);
	SPI1_Transmit(args[2]);
	SPI1_Transmit(args[3]);
	SPI1_Transmit(crc);
	SPI1_Transmit(0xff);

	uint8_t ret = SPI1_Transmit(0xff);
	SPI1_Transmit(0xff);
	SPI1_Deselect();

	return ret;
}


uint8_t SD_ReadSectors(uint8_t* buf, uint32_t sector, uint32_t count) {

	uint8_t args[4];

	args[0] = sector >> 24;
	args[1] = sector >> 16;
	args[2] = sector >> 8;
	args[3] = sector >> 0;

	uint8_t status;

	do {
		status = SD_SendCommand(SD_READ_MULTIPLE_BLOCK, args, 0xff);
		TIMER_Delay(5);
	} while (status != 0);

	return 0;
}

uint8_t SD_WriteSectors(uint8_t* buf, uint32_t sector, uint32_t count) {


	return 0;
}
