/*
 * arch/arm/mach-apple_iphone/include/mach/iphone-spi.h - SPI header for iPhone
 *
 * Copyright (C) 2008 Yiduo Wang
 *
 * Portions Copyright (C) 2010 Ricky Taylor
 *
 * This file is part of iDroid. An android distribution for Apple products.
 * For more information, please visit http://www.idroidproject.org/.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef IPHONE_SPI_H
#define IPHONE_SPI_H

#define GPIO_SPI0_CS0_IPHONE 0x400
#define GPIO_SPI0_CS0_IPOD 0x700

#ifdef CONFIG_IPODTOUCH_1G
#define GPIO_SPI2_CS0 0x1804
#define GPIO_SPI2_CS1 0x705
#endif

#ifdef CONFIG_IPHONE_2G
#define GPIO_SPI2_CS0 0x705
#endif

#ifdef CONFIG_IPODTOUCH_1G
#define GPIO_SPI0_CS0 GPIO_SPI0_CS0_IPOD
#else
#define GPIO_SPI0_CS0 GPIO_SPI0_CS0_IPHONE
#endif

#define GPIO_SPI1_CS0 0x1800

#ifdef CONFIG_IPHONE_3G
#define GPIO_SPI0_CS1 0x705
#define GPIO_SPI0_CS2 0x706
#endif

typedef enum SPIOption13 {
	SPIOption13Setting0 = 8,
	SPIOption13Setting1 = 16,
	SPIOption13Setting2 = 32
} SPIOption13;

void iphone_spi_set_baud(int port, int baud, SPIOption13 option13, bool isMaster, bool isActiveLow, bool lastClockEdgeMissing);
int iphone_spi_tx(int port, const u8* buffer, int len, bool block, bool unknown);
int iphone_spi_rx(int port, u8* buffer, int len, bool block, bool noTransmitJunk);
int iphone_spi_txrx(int port, const u8* outBuffer, int outLen, u8* inBuffer, int inLen, bool block);

#endif

