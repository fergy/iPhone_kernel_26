#include <linux/module.h>
#include <linux/interrupt.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/sched.h>

#include <mach/iphone-clock.h>
#include <mach/iphone-spi.h>

#define GET_BITS(x, start, length) ((((u32)(x)) << (32 - ((start) + (length)))) >> (32 - (length)))

// Device
#define CHIPID IO_ADDRESS(0x3E500000)

// Registers
#define SPICLOCKTYPE 0x4

// Values
#define GET_SPICLOCKTYPE(x) GET_BITS(x, 24, 4)

#define SPI0 IO_ADDRESS(0x3C300000)
#define SPI1 IO_ADDRESS(0x3CE00000)
#define SPI2 IO_ADDRESS(0x3D200000)

// Registers

#define CONTROL 0x0
#define SETUP 0x4
#define STATUS 0x8
#define UNKREG1 0xC
#define TXDATA 0x10
#define RXDATA 0x20
#define CLKDIVIDER 0x30
#define UNKREG2 0x34
#define UNKREG3 0x38

// Values
#define MAX_TX_BUFFER 8
#define TX_BUFFER_LEFT(x) GET_BITS(status, 4, 4)
#define RX_BUFFER_LEFT(x) GET_BITS(status, 8, 4)

#define CLOCK_SHIFT 12
#define MAX_DIVIDER 0x3FF

#define SPI0_CLOCKGATE 0x22
#define SPI1_CLOCKGATE 0x2B
#define SPI2_CLOCKGATE 0x2F

#define SPI0_IRQ 0x9
#define SPI1_IRQ 0xA
#define SPI2_IRQ 0xB

#define NUM_SPIPORTS 3

typedef struct SPIRegister {
	u32 control;
	u32 setup;
	u32 status;
	u32 unkReg1;
	u32 txData;
	u32 rxData;
	u32 clkDivider;
	u32 unkReg2;
	u32 unkReg3;
} SPIRegister;

typedef enum SPIClockSource {
	PCLK = 0,
	NCLK = 1
} SPIClockSource;

typedef struct SPIInfo {
	int option13;
	bool isActiveLow;
	bool lastClockEdgeMissing;
	SPIClockSource clockSource;
	int baud;
	bool isMaster;
	bool useDMA;
	const volatile u8* txBuffer;
	volatile int txCurrentLen;
	volatile int txTotalLen;
	volatile u8* rxBuffer;
	volatile int rxCurrentLen;
	volatile int rxTotalLen;
	volatile int counter;
	volatile bool txDone;
	volatile bool rxDone;

	struct completion complete;
} SPIInfo;

static const SPIRegister SPIRegs[NUM_SPIPORTS] = {
	{SPI0 + CONTROL, SPI0 + SETUP, SPI0 + STATUS, SPI0 + UNKREG1, SPI0 + TXDATA, SPI0 + RXDATA, SPI0 + CLKDIVIDER, SPI0 + UNKREG2, SPI0 + UNKREG3},
	{SPI1 + CONTROL, SPI1 + SETUP, SPI1 + STATUS, SPI1 + UNKREG1, SPI1 + TXDATA, SPI1 + RXDATA, SPI1 + CLKDIVIDER, SPI1 + UNKREG2, SPI1 + UNKREG3},
	{SPI2 + CONTROL, SPI2 + SETUP, SPI2 + STATUS, SPI2 + UNKREG1, SPI2 + TXDATA, SPI2 + RXDATA, SPI2 + CLKDIVIDER, SPI2 + UNKREG2, SPI2 + UNKREG3}
};

static SPIInfo spi_info[NUM_SPIPORTS];

static irqreturn_t spiIRQHandler(int irq, void* pPort);

int __init iphone_spi_setup(void)
{
	int i;
	int ret;

	iphone_clock_gate_switch(SPI0_CLOCKGATE, 1);
	iphone_clock_gate_switch(SPI1_CLOCKGATE, 1);
	iphone_clock_gate_switch(SPI2_CLOCKGATE, 1);

	memset(spi_info, 0, sizeof(SPIInfo) * NUM_SPIPORTS);

	for(i = 0; i < NUM_SPIPORTS; i++)
	{
		spi_info[i].clockSource = NCLK;
		init_completion(&spi_info[i].complete);
		writel(0, SPIRegs[i].control);
	}

        ret = request_irq(SPI0_IRQ, spiIRQHandler, IRQF_DISABLED, "iphone_spi", (void*) 0);
	if(ret)
		return ret;

        ret = request_irq(SPI1_IRQ, spiIRQHandler, IRQF_DISABLED, "iphone_spi", (void*) 1);
	if(ret)
		return ret;

        ret = request_irq(SPI2_IRQ, spiIRQHandler, IRQF_DISABLED, "iphone_spi", (void*) 2);
	if(ret)
		return ret;

	return 0;
}
module_init(iphone_spi_setup);

static int chipid_spi_clocktype(void)
{
	return GET_SPICLOCKTYPE(readl(CHIPID + SPICLOCKTYPE));
}

void iphone_spi_set_baud(int port, int baud, SPIOption13 option13, bool isMaster, bool isActiveLow, bool lastClockEdgeMissing)
{
	u32 clockFrequency;
	u32 divider;
	u32 options;

	if(port > (NUM_SPIPORTS - 1))
		return;

	writel(0, SPIRegs[port].control);

	switch(option13)
	{
		case SPIOption13Setting0:
			spi_info[port].option13 = 0;
			break;

		case SPIOption13Setting1:
			spi_info[port].option13 = 1;
			break;

		case SPIOption13Setting2:
			spi_info[port].option13 = 2;
			break;
	}

	spi_info[port].isActiveLow = isActiveLow;
	spi_info[port].lastClockEdgeMissing = lastClockEdgeMissing;

	if(spi_info[port].clockSource == PCLK)
	{
		clockFrequency = FREQUENCY_PERIPHERAL;
	} else
	{
		clockFrequency = FREQUENCY_FIXED;
	}

	if(chipid_spi_clocktype() != 0)
	{
		divider = clockFrequency / baud;
		if(divider < 2)
			divider = 2;
	} else
	{
		divider = clockFrequency / (baud * 2 - 1);
	}

	if(divider > MAX_DIVIDER)
	{
		return;
	}

	writel(divider, SPIRegs[port].clkDivider);
	spi_info[port].baud = baud;
	spi_info[port].isMaster = isMaster;

	options = (lastClockEdgeMissing << 1)
			| (isActiveLow << 2)
			| ((isMaster ? 0x3 : 0) << 3)
			| ((spi_info[port].useDMA ? 0x2 : 0x3D) << 5)
			| (spi_info[port].clockSource << CLOCK_SHIFT)
			| spi_info[port].option13 << 13;

	writel(options, SPIRegs[port].setup);
	writel(0, SPIRegs[port].unkReg1);
	writel(1, SPIRegs[port].control);

}

void wait_for_ready(int port)
{
	while(GET_BITS(readl(SPIRegs[port].status), 4, 4) != 0)
	{
		yield();
	}
}

int iphone_spi_tx(int port, const u8* buffer, int len, bool block, bool unknown)
{
	int i;

	if(port > (NUM_SPIPORTS - 1))
		return -1;

	writel(readl(SPIRegs[port].control) | (1 << 2), SPIRegs[port].control);
	writel(readl(SPIRegs[port].control) | (1 << 3), SPIRegs[port].control);

	spi_info[port].txBuffer = buffer;

	if(len > MAX_TX_BUFFER)
		spi_info[port].txCurrentLen = MAX_TX_BUFFER;
	else
		spi_info[port].txCurrentLen = len;

	spi_info[port].txTotalLen = len;
	spi_info[port].txDone = false;

	if(!unknown)
	{
		writel(0, SPIRegs[port].unkReg2);
	}

	for(i = 0; i < spi_info[port].txCurrentLen; i++)
	{
		writel(buffer[i], SPIRegs[port].txData);
	}

	INIT_COMPLETION(spi_info[port].complete);

	writel(1, SPIRegs[port].control);

	if(block)
	{
		wait_for_completion(&spi_info[port].complete);
		wait_for_ready(port);
		return len;
	} else
	{
		return 0;
	}
}

int iphone_spi_rx(int port, u8* buffer, int len, bool block, bool noTransmitJunk)
{
	if(port > (NUM_SPIPORTS - 1))
		return -1;

	writel(readl(SPIRegs[port].control) | (1 << 2), SPIRegs[port].control);
	writel(readl(SPIRegs[port].control) | (1 << 3), SPIRegs[port].control);

	spi_info[port].rxBuffer = buffer;
	spi_info[port].rxDone = false;
	spi_info[port].rxCurrentLen = 0;
	spi_info[port].rxTotalLen = len;
	spi_info[port].counter = 0;

	if(!noTransmitJunk) {
		writel(readl(SPIRegs[port].setup) | 1, SPIRegs[port].setup);
	}

	writel(len, SPIRegs[port].unkReg2);

	INIT_COMPLETION(spi_info[port].complete);

	writel(1, SPIRegs[port].control);

	if(block)
	{
		wait_for_completion(&spi_info[port].complete);

		if(!noTransmitJunk)
			writel(readl(SPIRegs[port].setup) & ~1, SPIRegs[port].setup);

		return len;
	} else {
		return 0;
	}
}

int iphone_spi_txrx(int port, const u8* outBuffer, int outLen, u8* inBuffer, int inLen, bool block)
{
	int i;

	if(port > (NUM_SPIPORTS - 1))
		return -1;

	writel(readl(SPIRegs[port].control) | (1 << 2), SPIRegs[port].control);
	writel(readl(SPIRegs[port].control) | (1 << 3), SPIRegs[port].control);

	spi_info[port].txBuffer = outBuffer;

	if(outLen > MAX_TX_BUFFER)
		spi_info[port].txCurrentLen = MAX_TX_BUFFER;
	else
		spi_info[port].txCurrentLen = outLen;

	spi_info[port].txTotalLen = outLen;
	spi_info[port].txDone = false;

	spi_info[port].rxBuffer = inBuffer;
	spi_info[port].rxDone = false;
	spi_info[port].rxCurrentLen = 0;
	spi_info[port].rxTotalLen = inLen;
	spi_info[port].counter = 0;

	for(i = 0; i < spi_info[port].txCurrentLen; i++)
		writel(outBuffer[i], SPIRegs[port].txData);

	writel(inLen, SPIRegs[port].unkReg2);

	INIT_COMPLETION(spi_info[port].complete);

	writel(1, SPIRegs[port].control);

	if(block)
	{
		wait_for_completion(&spi_info[port].complete);
		wait_for_ready(port);
		return inLen;
	} else
	{
		return 0;
	}
}

static irqreturn_t spiIRQHandler(int irq, void* pPort)
{
	int i;
	u32 status;
	int port = (int)pPort;

	if(port > (NUM_SPIPORTS - 1))
		return IRQ_HANDLED;

	status = readl(SPIRegs[port].status);
	if(status & (1 << 3))
		spi_info[port].counter++;

	if(status & (1 << 1))
	{
		while(true)
		{
			// take care of tx
			if(spi_info[port].txBuffer != NULL)
			{
				if(spi_info[port].txCurrentLen < spi_info[port].txTotalLen)
				{
					int toTX = spi_info[port].txTotalLen - spi_info[port].txCurrentLen;
					int canTX = MAX_TX_BUFFER - TX_BUFFER_LEFT(status);

					if(toTX > canTX)
						toTX = canTX;

					for(i = 0; i < toTX; i++)
					{
						writel(spi_info[port].txBuffer[spi_info[port].txCurrentLen + i], SPIRegs[port].txData);
					}

					spi_info[port].txCurrentLen += toTX;

				} else
				{
					spi_info[port].txDone = true;
					spi_info[port].txBuffer = NULL;
				}
			}

dorx:
			// take care of rx
			if(spi_info[port].rxBuffer == NULL)
				break;

			{
				int toRX = spi_info[port].rxTotalLen - spi_info[port].rxCurrentLen;
				int canRX = GET_BITS(status, 8, 4);

				if(toRX > canRX)
					toRX = canRX;

				for(i = 0; i < toRX; i++)
				{
					spi_info[port].rxBuffer[spi_info[port].rxCurrentLen + i] = readl(SPIRegs[port].rxData);
				}

				spi_info[port].rxCurrentLen += toRX;

				if(spi_info[port].rxCurrentLen < spi_info[port].rxTotalLen)
					break;

				spi_info[port].rxDone = true;
				spi_info[port].rxBuffer = NULL;
			}

		}


	} else  if(status & (1 << 0))
	{
		// jump into middle of the loop to handle rx only, stupidly
		goto dorx;
	}

	// acknowledge interrupt handling complete
	writel(status, SPIRegs[port].status);

	if((!spi_info[port].rxBuffer || spi_info[port].rxDone) && (!spi_info[port].txBuffer || spi_info[port].txDone))
		complete(&spi_info[port].complete);

	return IRQ_HANDLED;
}

