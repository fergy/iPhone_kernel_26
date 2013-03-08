#ifndef IPHONE_HW_NAND_H
#define IPHONE_HW_NAND_H
#include <mach/hardware.h>

// Device
#define NAND IO_ADDRESS(0x38A00000)
#define NANDECC IO_ADDRESS(0x38F00000)
#define NAND_CLOCK_GATE1 0x8
#define NAND_CLOCK_GATE2 0xC
#define NANDECC_INT 0x2B


// Registers
#define NAND_CONFIG 0x0
#define NAND_CON 0x4
#define NAND_CMD 0x8
#define NAND_CONFIG3 0xC
#define NAND_CONFIG4 0x2C
#define NAND_CONFIG5 0x10
#define NAND_TRANSFERSIZE 0x30
#define NAND_CONFIG6 0x44
#define NAND_STATUS 0x48
#define NAND_DMA_SOURCE 0x80
#define NAND_SETUP 0x100

#define NANDECC_DATA 0x4
#define NANDECC_ECC 0x8
#define NANDECC_START 0xC
#define NANDECC_STATUS 0x10
#define NANDECC_SETUP 0x14
#define NANDECC_CLEARINT 0x40

// Values

#define NAND_CONFIG_DEFAULTS 0x801
#define NAND_CONFIG_SETTING1SHIFT 12
#define NAND_CONFIG_SETTING2SHIFT 16
#define NAND_CONFIG_SETTING1MASK 0x7
#define NAND_CONFIG_SETTING2MASK 0x7
#define NAND_CONFIG_DMASETTINGSHIFT 10

#define NAND_CMD_RESET 0xFF
#define NAND_CMD_ID 0x90
#define NAND_CMD_READSTATUS 0x70
#define NAND_CMD_READ 0x30

#define NAND_CONFIG4_TRANSFERSETTING 4

#define NAND_CON_ADDRESSDONE (1 << 0)
#define NAND_CON_SETTING1 0xC0
#define NAND_CON_SETTING2 0x80
#define NAND_CON_BEGINTRANSFER (1 << 1)

#define NAND_STATUS_READY 0x1

#define NAND_NUM_BANKS 8

#endif

