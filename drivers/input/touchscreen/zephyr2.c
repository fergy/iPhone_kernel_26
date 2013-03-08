#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/firmware.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <mach/iphone-spi.h>
#include <mach/gpio.h>

#ifdef CONFIG_IPHONE_2G
#	define MT_GPIO_POWER 0x804
#	define MT_ATN_INTERRUPT 0xa3
#else
#	define MT_GPIO_POWER 0x701
#	define MT_ATN_INTERRUPT 0x9b
#endif

#ifdef CONFIG_IPHONE_3G
#	define MT_SPI 1
#	define MT_SPI_CS GPIO_SPI1_CS0
#else
#	define MT_SPI 2
#	define MT_SPI_CS GPIO_SPI2_CS0
#endif

#define MT_INFO_FAMILYID 0xD1
#define MT_INFO_SENSORINFO 0xD3
#define MT_INFO_SENSORREGIONDESC 0xD0
#define MT_INFO_SENSORREGIONPARAM 0xA1
#define MT_INFO_SENSORDIM 0xD9

#define MAX_FINGER_ORIENTATION  16384
#define MAX_BUFFER_SIZE 0x400

#define NORMAL_SPEED (&MTNormalSpeed)
#define FAST_SPEED (&MTFastSpeed)

typedef struct MTFrameHeader
{
	u8 type;
	u8 frameNum;
	u8 headerLen;
	u8 unk_3;
	u32 timestamp;
	u8 unk_8;
	u8 unk_9;
	u8 unk_A;
	u8 unk_B;
	u16 unk_C;
	u16 isImage;

	u8 numFingers;
	u8 fingerDataLen;
	u16 unk_12;
	u16 unk_14;
	u16 unk_16;
} MTFrameHeader;

typedef struct FingerData
{
	u8 id;
	u8 event;
	u8 unk_2;
	u8 unk_3;
	s16 x;
	s16 y;
	s16 rel_x;
	s16 rel_y;
	u16 size_major;
	u16 size_minor;
	u16 orientation;
	u16 force_major;
	u16 force_minor;
	u16 unk_16;
	u16 unk_18;
	u16 unk_1A;
} FingerData;

typedef struct MTSPISetting
{
	int speed;
	int txDelay;
	int rxDelay;
} MTSPISetting;

const MTSPISetting MTNormalSpeed = {83000, 5, 10};
const MTSPISetting MTFastSpeed = {4500000, 0, 10};

static u8* OutputPacket;
static u8* InputPacket;
static u8* GetInfoPacket;
static u8* GetResultPacket;

static int InterfaceVersion;
static int MaxPacketSize;
static int FamilyID;
static int FlipNOP;
static int SensorWidth;
static int SensorHeight;
static int SensorColumns;
static int SensorRows;
static int BCDVersion;
static int Endianness;
static u8* SensorRegionDescriptor;
static int SensorRegionDescriptorLen;
static u8* SensorRegionParam;
static int SensorRegionParamLen;

static int CurNOP;

static bool FirmwareLoaded = false;
static int z2_atn_count = 0;

static struct device* multitouch_dev = NULL;
static struct input_dev* input_dev;

static u8* constructed_fw;
static size_t constructed_fw_size;
static u8 proxcal_fw[512];
static size_t proxcal_fw_size;
static u8 cal_fw[512];
static size_t cal_fw_size;

static int makeBootloaderDataPacket(u8* output, u32 destAddress, const u8* data, int dataLen, int* cksumOut);
static void sendExecutePacket(void);

static bool loadConstructedFirmware(const u8* firmware, int len);
static int loadProxCal(const u8* firmware, int len);
static int loadCal(const u8* firmware, int len);
static bool determineInterfaceVersion(void);

static bool getReportInfo(int id, u8* err, u16* len);
static bool getReport(int id, u8* buffer, int* outLen);

static void newPacket(const u8* data, int len);

static u32 z2_getU32(u8 *_buf)
{
	return (_buf[2] << 24)
		| (_buf[3] << 16)
		| (_buf[0] << 8)
		| _buf[1];
}

static void z2_makeU32(u8 *_buf, u32 _val)
{
	_buf[2] = _val >> 24;
	_buf[3] = (_val >> 16) & 0xFF;
	_buf[0] = (_val >> 8) & 0xFF;
	_buf[1] = _val & 0xFF;
}
static u16 z2_getU16(u8 *_buf)
{
	return (_buf[0] << 8)
		| _buf[1];
}

static void z2_makeU16(u8 *_buf, u16 _val)
{
	_buf[0] = _val >> 8;
	_buf[1] = _val & 0xFF;
}

static u16 z2_getU16R(u8 *_buf)
{
	return (_buf[1] << 8)
		| _buf[0];
}

static void z2_makeU16R(u8 *_buf, u16 _val)
{
	_buf[1] = _val >> 8;
	_buf[0] = _val & 0xFF;
}

static u32 z2_u32Sum(u8 *_buf, u32 _len)
{
	u32 i;
	u32 checksum = 0;

	for(i = 0; i < _len; i++)
	{
		checksum += _buf[i];
	}

	return checksum;
}

static u16 z2_u16Sum(u8 *_buf, u32 _len)
{
	u32 i;
	u16 checksum = 0;

	for(i = 0; i < _len; i++)
	{
		checksum += _buf[i];
	}

	return checksum;
}

static u16 z2_makeU16Sum(u8 *_buf, u32 _len)
{
	u16 chk = z2_u16Sum(_buf, _len);
	z2_makeU16(_buf+_len, chk);
	return chk;
}

static u16 z2_makeU16SumR(u8 *_buf, u32 _len)
{
	u16 chk = z2_u16Sum(_buf, _len);
	z2_makeU16R(_buf+_len, chk);
	return chk;
}

static u32 z2_makeU32Sum(u8 *_buf, u32 _len)
{
	u32 chk = z2_u32Sum(_buf, _len);
	z2_makeU32(_buf+_len, chk);
	return chk;
}

int z2_tx(const MTSPISetting* setting, const u8* outBuffer, int outLen)
{
	int ret;
	iphone_spi_set_baud(MT_SPI, setting->speed, SPIOption13Setting0, 1, 1, 1);
	iphone_gpio_pin_output(MT_SPI_CS, 0);
	msleep(setting->txDelay);
	ret = iphone_spi_tx(MT_SPI, outBuffer, outLen, true, false);
	iphone_gpio_pin_output(MT_SPI_CS, 1);
	return ret;
}

int z2_txrx(const MTSPISetting* setting, const u8* outBuffer, int outLen, u8* inBuffer, int inLen)
{
	int ret;
	iphone_spi_set_baud(MT_SPI, setting->speed, SPIOption13Setting0, 1, 1, 1);
	iphone_gpio_pin_output(MT_SPI_CS, 0);
	msleep(setting->rxDelay);
	ret = iphone_spi_txrx(MT_SPI, outBuffer, outLen, inBuffer, inLen, true);
	iphone_gpio_pin_output(MT_SPI_CS, 1);
	return ret;
}

static u16 z2_shortAck(void)
{
	u8 tx[2];
	u8 rx[2];

	z2_makeU16(tx, 0x1aa1);

	//while(GotATN == 0);
	//	--GotATN;

	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	return z2_getU16(rx);
}

static u32 z2_longAck(void)
{
	u8 tx[8];
	u8 rx[8];

	//while(GotATN == 0);
	//	--GotATN;

	z2_makeU16(tx, 0x1aa1);

	z2_makeU16(tx+2, 0x18e1);
	z2_makeU16(tx+4, 0x18e1);
	z2_makeU16(tx+6, 0x18e1);

	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	return z2_getU32(rx+2);
}

static u32 readRegister(u32 address)
{
	u8 tx[8];
	u8 rx[8];

	z2_makeU16(tx, 0x1c73);
	z2_makeU32(tx+2, address);
	z2_makeU16Sum(tx+2, 4);

	//GotATN = 0;
	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));
	udelay(300);

	return z2_longAck();
}

static u32 writeRegister(u32 address, u32 value, u32 mask)
{
	u8 tx[16];
	u8 rx[16];

	z2_makeU16(tx, 0x1e33);
	z2_makeU32(tx+2, address);
	z2_makeU32(tx+6, mask);
	z2_makeU32(tx+10, value);
	z2_makeU16Sum(tx+2, sizeof(u32)*3);

	//GotATN = 0;
	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));
	udelay(300);

	return z2_shortAck() == 0x4AD1;
}


static void newPacket(const u8* data, int len)
{
	int i;
	FingerData* finger;
	MTFrameHeader* header = (MTFrameHeader*) data;
	if(header->type != 0x44 && header->type != 0x43)
		printk("zephyr2: unknown frame type 0x%x\n", header->type);

	finger = (FingerData*)(data + (header->headerLen));

	if(header->headerLen < 12)
		printk("zephyr2: no finger data in frame\n");

	//	printk("------START------\n");

	for(i = 0; i < header->numFingers; ++i)
	{
		input_report_abs(input_dev, ABS_MT_TOUCH_MAJOR, finger->force_major);
		input_report_abs(input_dev, ABS_MT_TOUCH_MINOR, finger->force_minor);
		input_report_abs(input_dev, ABS_MT_WIDTH_MAJOR, finger->size_major);
		input_report_abs(input_dev, ABS_MT_WIDTH_MINOR, finger->size_minor);
		input_report_abs(input_dev, ABS_MT_ORIENTATION, MAX_FINGER_ORIENTATION - finger->orientation);
		input_report_abs(input_dev, ABS_MT_POSITION_X, finger->x);
		input_report_abs(input_dev, ABS_MT_POSITION_Y, SensorHeight - finger->y);
		input_report_abs(input_dev, ABS_MT_TRACKING_ID, finger->id);
		input_mt_sync(input_dev);
		/*printk("zephyr2: finger %d -- id=%d, event=%d, X(%d/%d, vel: %d), Y(%d/%d, vel: %d), radii(%d, %d, %d, orientation: %d), force_minor: %d\n",
				i, finger->id, finger->event,
				finger->x, SensorWidth, finger->rel_x,
				finger->y, SensorHeight, finger->rel_y,
				finger->force_major, finger->size_major, finger->size_minor, finger->orientation,
				finger->force_minor);

		//framebuffer_draw_rect(0xFF0000, (finger->x * framebuffer_width()) / SensorWidth - 2 , ((SensorHeight - finger->y) * framebuffer_height()) / SensorHeight - 2, 4, 4);
		//hexdump((u32) finger, sizeof(FingerData));*/
		finger = (FingerData*) (((u8*) finger) + header->fingerDataLen);
	}

	if(header->numFingers > 0)
	{
		finger = (FingerData*)(data + (header->headerLen));

		input_report_abs(input_dev, ABS_X, finger->x);
		input_report_abs(input_dev, ABS_Y, SensorHeight - finger->y);
		input_report_key(input_dev, BTN_TOUCH, finger->size_minor > 0);
	}

	input_sync(input_dev);

	//	printk("-------END-------\n");
}

static bool readResultData(int len)
{
	u32 checksum;
	int i;
	int packetLen;

	if(len > MAX_BUFFER_SIZE)
	{
		printk("zephyr2: Result too big for buffer! We have %d bytes, we need %d bytes!\n", MAX_BUFFER_SIZE, len);
		len = MAX_BUFFER_SIZE;
	}

	memset(GetResultPacket, 0, MAX_BUFFER_SIZE);

	if(FlipNOP)
		GetResultPacket[0] = 0xEB;
	else
		GetResultPacket[0] = 0xEA;

	GetResultPacket[1] = CurNOP;
	GetResultPacket[2] = 1;

	checksum = 0;
	for(i = 0; i < 14; ++i)
		checksum += GetResultPacket[i];

	GetResultPacket[len - 2] = checksum & 0xFF;
	GetResultPacket[len - 1] = (checksum >> 8) & 0xFF;

	z2_txrx(NORMAL_SPEED, GetResultPacket, len, InputPacket, len);

	if(InputPacket[0] != 0xEA && !(FlipNOP && InputPacket[0] == 0xEB))
	{
		printk("zephyr2: frame header wrong: got 0x%02X\n", InputPacket[0]);
		msleep(1);
		return false;
	}

	checksum = 0;
	for(i = 0; i < 5; ++i)
		checksum += InputPacket[i];

	if((checksum & 0xFF) != 0)
	{
		printk("zephyr2: LSB of first five bytes of frame not zero: got 0x%02X\n", checksum);
		msleep(1);
		return false;
	}

	packetLen = (InputPacket[2] & 0xFF) | ((InputPacket[3] & 0xFF) << 8);

	if(packetLen <= 2)
		return true;

	checksum = 0;
	for(i = 5; i < (5 + packetLen - 2); ++i)
		checksum += InputPacket[i];
	if((InputPacket[len - 2] | (InputPacket[len - 1] << 8)) != checksum)
	{
		printk("zephyr2: packet checksum wrong 0x%02X instead of 0x%02X\n", checksum, (InputPacket[len - 2] | (InputPacket[len - 1] << 8)));
		msleep(1);
		return false;
	}

	newPacket(InputPacket + 5, packetLen - 2);
	return true;
}

static bool z2_readFrameLength(int* len)
{
	u8 tx[16];
	u8 rx[16];
	u32 checksum;

	memset(tx, 0, sizeof(tx));

	if(FlipNOP)
		tx[0] = 0xEB;
	else
		tx[0] = 0xEA;

	tx[1] = CurNOP;
	tx[2] = 0;

	z2_makeU16SumR(tx, 14);

	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	checksum = z2_u16Sum(rx, 14);
	if((rx[14] | (rx[15] << 8)) != checksum)
	{
		udelay(1000);
		return false;
	}

	*len = (rx[1] & 0xFF) | ((rx[2] & 0xFF) << 8);

	return true;
}

static int z2_readFrame(void)
{
	int ret = 0;
	int len = 0;

	if(!z2_readFrameLength(&len))
	{
		printk("zephyr2: error getting frame length\r\n");
		len = 0;
		ret = -1;
	}

	if(len)
	{
		if(!readResultData(len + 5))
		{
			printk("zephyr2: error getting frame data\r\n");
			msleep(1);
			ret = -1;
		}

		ret = 1;
	}

	if(FlipNOP)
	{
		if(CurNOP == 1)
			CurNOP = 2;
		else
			CurNOP = 1;
	}

	return ret;
}


static int shortControlRead(int id, u8* buffer, int size)
{
	u32 checksum;
	int i;

	memset(GetInfoPacket, 0, MAX_BUFFER_SIZE);
	GetInfoPacket[0] = 0xE6;
	GetInfoPacket[1] = id;
	GetInfoPacket[2] = 0;
	GetInfoPacket[3] = size & 0xFF;
	GetInfoPacket[4] = (size >> 8) & 0xFF;

	checksum = 0;
	for(i = 0; i < 5; ++i)
		checksum += GetInfoPacket[i];

	GetInfoPacket[14] = checksum & 0xFF;
	GetInfoPacket[15] = (checksum >> 8) & 0xFF;

	z2_txrx(NORMAL_SPEED, GetInfoPacket, 16, InputPacket, 16);

	udelay(25);

	GetInfoPacket[2] = 1;

	z2_txrx(NORMAL_SPEED, GetInfoPacket, 16, InputPacket, 16);

	checksum = 0;
	for(i = 0; i < 14; ++i)
		checksum += InputPacket[i];

	if((InputPacket[14] | (InputPacket[15] << 8)) != checksum)
		return false;

	memcpy(buffer, &InputPacket[3], size);

	return true;
}

static int longControlRead(int id, u8* buffer, int size)
{
	u32 checksum;
	int i;

	memset(GetInfoPacket, 0, 0x200);
	GetInfoPacket[0] = 0xE7;
	GetInfoPacket[1] = id;
	GetInfoPacket[2] = 0;
	GetInfoPacket[3] = size & 0xFF;
	GetInfoPacket[4] = (size >> 8) & 0xFF;

	checksum = 0;
	for(i = 0; i < 5; ++i)
		checksum += GetInfoPacket[i];

	GetInfoPacket[14] = checksum & 0xFF;
	GetInfoPacket[15] = (checksum >> 8) & 0xFF;

	z2_txrx(NORMAL_SPEED, GetInfoPacket, 16, InputPacket, 16);

	udelay(25);

	GetInfoPacket[2] = 1;
	GetInfoPacket[14] = 0;
	GetInfoPacket[15] = 0;
	GetInfoPacket[3 + size] = checksum & 0xFF;
	GetInfoPacket[3 + size + 1] = (checksum >> 8) & 0xFF;

	z2_txrx(NORMAL_SPEED, GetInfoPacket, size + 5, InputPacket, size + 5);

	checksum = 0;
	for(i = 0; i < (size + 3); ++i)
		checksum += InputPacket[i];

	if((InputPacket[3 + size] | (InputPacket[3 + size + 1] << 8)) != checksum)
		return false;

	memcpy(buffer, &InputPacket[3], size);

	return true;
}

static bool getReportInfo(int id, u8* err, u16* len)
{
	u8 tx[16];
	u8 rx[16];
	u32 checksum;
	int i;
	int try;

	for(try = 0; try < 4; ++try)
	{
		memset(tx, 0, sizeof(tx));

		tx[0] = 0xE3;
		tx[1] = id;

		checksum = 0;
		for(i = 0; i < 14; ++i)
			checksum += tx[i];

		tx[14] = checksum & 0xFF;
		tx[15] = (checksum >> 8) & 0xFF;

		z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

		udelay(25);

		z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

		if(rx[0] != 0xE3)
			continue;

		checksum = 0;
		for(i = 0; i < 14; ++i)
			checksum += rx[i];

		if((rx[14] | (rx[15] << 8)) != checksum)
			continue;

		*err = rx[2];
		*len = (rx[4] << 8) | rx[3];

		return true;
	}

	return false;
}

static bool getReport(int id, u8* buffer, int* outLen)
{
	u8 err;
	u16 len;
	int try;

	if(!getReportInfo(id, &err, &len))
		return false;

	if(err)
		return false;

	*outLen = len;

	for(try = 0; try < 4; ++try)
	{
		if(len < 12)
		{
			if(shortControlRead(id, buffer, len))
				return true;
		} else
		{
			if(longControlRead(id, buffer, len))
				return true;
		}
	}

	return false;
}

static bool determineInterfaceVersion(void)
{
	u8 tx[16];
	u8 rx[16];
	u32 checksum;
	int i;
	int try;

	memset(tx, 0, sizeof(tx));

	tx[0] = 0xE2;

	checksum = 0;
	for(i = 0; i < 14; ++i)
		checksum += tx[i];

	// Note that the byte order changes to little-endian after main firmware load

	tx[14] = checksum & 0xFF;
	tx[15] = (checksum >> 8) & 0xFF;

	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	for(try = 0; try < 4; ++try)
	{
		z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

		if(rx[0] == 0xE2)
		{
			checksum = 0;
			for(i = 0; i < 14; ++i)
				checksum += rx[i];

			if((rx[14] | (rx[15] << 8)) == checksum)
			{
				InterfaceVersion = rx[2];
				MaxPacketSize = (rx[4] << 8) | rx[3];
				printk("zephyr2: interface version %d, max packet size: %d\n", InterfaceVersion, MaxPacketSize);

				return true;
			}
		}

		InterfaceVersion = 0;
		MaxPacketSize = 1000;
		msleep(3);
	}

	printk("zephyr2: failed getting interface version!\n");

	return false;
}

static bool loadConstructedFirmware(const u8* firmware, int len)
{
	int try;

	for(try = 0; try < 5; ++try)
	{

		printk("zephyr2: uploading firmware\n");

		//                GotATN = 0;
		z2_tx(FAST_SPEED, firmware, len);

		udelay(300);

		if(z2_shortAck() == 0x4BC1)
			return true;

	}

	return false;
}

static int loadProxCal(const u8* firmware, int len)
{
	u32 address = 0x400180;
	const u8* data = firmware;
	int left = (len + 3) & ~0x3;
	int try;

	while(left > 0)
	{
		int toUpload = left;
		if(toUpload > (MAX_BUFFER_SIZE - 0x10))
		{
			printk("zephyr2: prox-cal too big for buffer.\n");
			toUpload = MAX_BUFFER_SIZE - 0x10;
		}

		OutputPacket[0] = 0x18;
		OutputPacket[1] = 0xE1;

		makeBootloaderDataPacket(OutputPacket + 2, address, data, toUpload, NULL);

		for(try = 0; try < 5; ++try)
		{
			printk("zephyr2: uploading prox calibration data packet\r\n");

			//                        GotATN = 0;
			z2_tx(FAST_SPEED, OutputPacket, toUpload + 0x10);
			udelay(300);

			if(z2_shortAck() == 0x4BC1)
				break;
		}

		if(try == 5)
			return false;

		address += toUpload;
		data += toUpload;
		left -= toUpload;
	}

	return true;
}

static int loadCal(const u8* firmware, int len)
{
	u32 address = 0x400200;
	const u8* data = firmware;
	int left = (len + 3) & ~0x3;
	int try;

	while(left > 0)
	{
		int toUpload = left;
		if(toUpload > 0x3F0)
			toUpload = 0x3F0;

		OutputPacket[0] = 0x18;
		OutputPacket[1] = 0xE1;

		makeBootloaderDataPacket(OutputPacket + 2, address, data, toUpload, NULL);

		for(try = 0; try < 5; ++try)
		{
			printk("zephyr2: uploading calibration data packet\r\n");

			//                        GotATN = 0;
			z2_tx(FAST_SPEED, OutputPacket, toUpload + 0x10);
			udelay(300);

			if(z2_shortAck() == 0x4BC1)
				break;
		}

		if(try == 5)
			return false;

		address += toUpload;
		data += toUpload;
		left -= toUpload;
	}

	return true;
}

static u32 z2_getCalibration(void)
{
	u8 tx[2];
	u8 rx[2];

	tx[0] = 0x1F;
	tx[1] = 0x01;

	printk("zephyr2: requesting calibration...\n");

	//	GotATN = 0;
	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	msleep(65);

	return z2_longAck();
}

static int z2_calibrate(void)
{
	u32 z2_version;

	z2_version = readRegister(0x10008FFC);

	printk("zephyr2: detected Zephyr2 version 0x%0X\n", z2_version);

	if(z2_version != 0x5A020028 && z2_version != 0x5A02002A) // TODO: What the hell causes this to crash? ;_;
	{
#define init_register(addr, a, b) if(!writeRegister(addr, (a), (b))) { \
	printk("zephyr2: error initialising register " #addr "\n"); return false; }

		printk("zephyr2: Initialising Registers...\n");
		// -- BEGIN INITIALISING REGISTERS -- //

		init_register(0x10001C04, 0x16E4, 0x1FFF);
		init_register(0x10001C08, 0x840000, 0xFF0000);
		init_register(0x1000300C, 0x05, 0x85);
		init_register(0x1000304C, 0x20, 0xFFFFFFFF);

		// --- END INITIALISING REGISTERS --- //
		printk("zephyr2: Initialised Registers\n");

#undef init_register
	}

	printk("zephyr2: calibration complete with 0x%x\n", z2_getCalibration());

	return true;
}

static void sendExecutePacket(void)
{
	u8 tx[12];
	u8 rx[12];

	tx[0] = 0x1D;
	tx[1] = 0x53;

	tx[2] = 0x18;
	tx[3] = 0x00;
	tx[4] = 0x10;
	tx[5] = 0x00;
	tx[6] = 0x00;
	tx[7] = 0x01;
	tx[8] = 0x00;
	tx[9] = 0x00;

	z2_makeU16Sum(tx+2, 8);
	z2_txrx(NORMAL_SPEED, tx, sizeof(tx), rx, sizeof(rx));

	printk("zephyr2: execute packet sent\r\n");
}

static int makeBootloaderDataPacket(u8* output, u32 destAddress, const u8* data, int dataLen, int* cksumOut)
{
	u32 checksum;
	int i;

	// This seems to be middle-endian! I've never seen this before.

	output[0] = 0x30;
	output[1] = 0x01;
	z2_makeU16(output+2, dataLen >> 2);
	z2_makeU32(output+4, destAddress);
	z2_makeU16Sum(output+2, 6);

	for(i = 0; i < dataLen; i += 4)
	{
		output[10 + i + 0] = data[i + 1];
		output[10 + i + 1] = data[i + 0];
		output[10 + i + 2] = data[i + 3];
		output[10 + i + 3] = data[i + 2];
	}

	checksum = z2_makeU32Sum(output+10, dataLen);

	if(cksumOut)
		*cksumOut = checksum;

	return dataLen;
}

static spinlock_t z2_atn_count_lock;

static void z2_atn_handler(struct work_struct* work)
{
	unsigned long flags;

	spin_lock_irqsave(&z2_atn_count_lock, flags);
	while(z2_atn_count > 0)
	{
		--z2_atn_count;
		spin_unlock_irqrestore(&z2_atn_count_lock, flags);
		z2_readFrame();
		spin_lock_irqsave(&z2_atn_count_lock, flags);
	}
	spin_unlock_irqrestore(&z2_atn_count_lock, flags);
}
DECLARE_WORK(z2_queue, &z2_atn_handler);

static irqreturn_t z2_irq(int irq, void* pToken)
{
	unsigned long flags;

	if(!FirmwareLoaded)
		return IRQ_HANDLED;

	spin_lock_irqsave(&z2_atn_count_lock, flags);
	++z2_atn_count;
	spin_unlock_irqrestore(&z2_atn_count_lock, flags);

	schedule_work(&z2_queue);

	return IRQ_HANDLED;
}

int z2_setup(const u8* constructedFirmware, int constructedFirmwareLen, const u8* prox_cal, int prox_cal_size, const u8* cal, int cal_size)
{
	int i;
	int ret;
	int err;
	u8* reportBuffer;
	int reportLen;

	spin_lock_init(&z2_atn_count_lock);

	if(!prox_cal)
	{
		printk("zephyr2: could not find proximity calibration data\n");
		return -1;
	}

	if(!cal)
	{
		printk("zephyr2: could not find calibration data\n");
		return -1;
	}

	printk("zephyr2: Firmware at 0x%08x - 0x%08x\n",
			(u32) constructedFirmware, (u32)(constructedFirmware + constructedFirmwareLen));

	OutputPacket = (u8*) kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
	InputPacket = (u8*) kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
	GetInfoPacket = (u8*) kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
	GetResultPacket = (u8*) kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);

	request_irq(MT_ATN_INTERRUPT + IPHONE_GPIO_IRQS, z2_irq, IRQF_TRIGGER_FALLING, "iphone-multitouch", (void*) 0);

	// Power up the device (turn it off then on again. ;])
	printk("zephyr2: Powering Up Multitouch!\n");
	iphone_gpio_pin_output(MT_GPIO_POWER, 0);
	msleep(200);

	iphone_gpio_pin_output(MT_GPIO_POWER, 1);
	msleep(15);

	for(i = 0; i < 4; ++i)
	{
		iphone_gpio_pin_output(0x606, 0);
		msleep(200);
		iphone_gpio_pin_output(0x606, 1);
		msleep(15);

		// Send Firmware
		printk("zephyr2: Sending Firmware...\n");
		if(loadConstructedFirmware(constructedFirmware, constructedFirmwareLen))
		{
			break;
		}
	}

	if(i == 4)
	{
			printk("zephyr2: could not load preconstructed firmware\n");
			err = -1;
			kfree(InputPacket);
			kfree(OutputPacket);
			kfree(GetInfoPacket);
			kfree(GetResultPacket);
			return err;
	}

	printk("zephyr2: loaded %d byte preconstructed firmware\n", constructedFirmwareLen);

#ifndef CONFIG_IPODTOUCH_1G
	if(!loadProxCal(prox_cal, prox_cal_size))
	{
		printk("zephyr2: could not load proximity calibration data\n");
		err = -1;
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	printk("zephyr2: loaded %d byte proximity calibration data\n", prox_cal_size);
#endif

	if(!loadCal(cal, cal_size))
	{
		printk("zephyr2: could not load calibration data\n");
		err = -1;
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}


	printk("zephyr2: loaded %d byte calibration data\n", cal_size);

	if(!z2_calibrate())
	{
		err = -1;
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	sendExecutePacket();

	msleep(1);

	printk("zephyr2: Determining interface version...\n");
	if(!determineInterfaceVersion())
	{
		err = -1;
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	reportBuffer = (u8*) kmalloc(MaxPacketSize, GFP_KERNEL);

	if(!getReport(MT_INFO_FAMILYID, reportBuffer, &reportLen))
	{
		printk("zephyr2: failed getting family id!\n");
		err = -1;
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	FamilyID = reportBuffer[0];

	if(!getReport(MT_INFO_SENSORINFO, reportBuffer, &reportLen))
	{
		printk("zephyr2: failed getting sensor info!\r\n");
		err = -1;
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	SensorColumns = reportBuffer[2];
	SensorRows = reportBuffer[1];
	BCDVersion = ((reportBuffer[3] & 0xFF) << 8) | (reportBuffer[4] & 0xFF);
	Endianness = reportBuffer[0];


	if(!getReport(MT_INFO_SENSORREGIONDESC, reportBuffer, &reportLen))
	{
		printk("zephyr2: failed getting sensor region descriptor!\r\n");
		err = -1;
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}


	SensorRegionDescriptorLen = reportLen;
	SensorRegionDescriptor = (u8*) kmalloc(reportLen, GFP_KERNEL);
	memcpy(SensorRegionDescriptor, reportBuffer, reportLen);

	if(!getReport(MT_INFO_SENSORREGIONPARAM, reportBuffer, &reportLen))
	{
		printk("zephyr2: failed getting sensor region param!\r\n");
		err = -1;
		kfree(SensorRegionDescriptor);
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}


	SensorRegionParamLen = reportLen;
	SensorRegionParam = (u8*) kmalloc(reportLen, GFP_KERNEL);
	memcpy(SensorRegionParam, reportBuffer, reportLen);

	if(!getReport(MT_INFO_SENSORDIM, reportBuffer, &reportLen))
	{
		printk("zephyr2: failed getting sensor surface dimensions!\r\n");
		err = -1;
		kfree(SensorRegionParam);
		kfree(SensorRegionDescriptor);
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}


	SensorWidth = *((u32*)&reportBuffer[0]);
	SensorHeight = *((u32*)&reportBuffer[4]);

	printk("Family ID                : 0x%x\n", FamilyID);
	printk("Sensor rows              : 0x%x\n", SensorRows);
	printk("Sensor columns           : 0x%x\n", SensorColumns);
	printk("Sensor width             : 0x%x\n", SensorWidth);
	printk("Sensor height            : 0x%x\n", SensorHeight);
	printk("BCD Version              : 0x%x\n", BCDVersion);
	printk("Endianness               : 0x%x\n", Endianness);
	printk("Sensor region descriptor :");

	for(i = 0; i < SensorRegionDescriptorLen; ++i)
		printk(" %02x", SensorRegionDescriptor[i]);
	printk("\n");

	printk("Sensor region param      :");
	for(i = 0; i < SensorRegionParamLen; ++i)
		printk(" %02x", SensorRegionParam[i]);
	printk("\n");

	if(BCDVersion > 0x23)
		FlipNOP = true;
	else
		FlipNOP = false;

	kfree(reportBuffer);


	input_dev = input_allocate_device();
	if(!input_dev)
	{
		err = -1;
		kfree(SensorRegionParam);
		kfree(SensorRegionDescriptor);
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	input_dev->name = "iPhone Zephyr 2 Multitouch Screen";
	input_dev->phys = "multitouch0";
	input_dev->id.vendor = 0x05AC;
	input_dev->id.product = 0;
	input_dev->id.version = 0x0000;
	input_dev->dev.parent = multitouch_dev;
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(input_dev, ABS_X, 0, SensorWidth, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, SensorHeight, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, max(SensorHeight, SensorWidth), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0, max(SensorHeight, SensorWidth), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, max(SensorHeight, SensorWidth), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MINOR, 0, max(SensorHeight, SensorWidth), 0, 0);
	input_set_abs_params(input_dev, ABS_MT_ORIENTATION, -MAX_FINGER_ORIENTATION, MAX_FINGER_ORIENTATION, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, SensorWidth, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, SensorHeight, 0, 0);

	/* not sure what the actual max is */
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 32, 0, 0);

	ret = input_register_device(input_dev);
	if(ret != 0)
	{
		err = -1;
		kfree(SensorRegionParam);
		kfree(SensorRegionDescriptor);
		kfree(reportBuffer);
		kfree(InputPacket);
		kfree(OutputPacket);
		kfree(GetInfoPacket);
		kfree(GetResultPacket);
		return err;
	}

	CurNOP = 1;

	//spin_lock_init(&z2_readFrame_lock);

	FirmwareLoaded = true;

	return 0;
}

static void got_constructed(const struct firmware* fw, void *context)
{
	if(!fw)
	{
		printk("zephyr2: couldn't get firmware, trying again...\n");
		request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG, "zephyr2.bin", multitouch_dev, NULL, got_constructed);
		return;
	}

	constructed_fw = kmalloc(fw->size, GFP_KERNEL);
	constructed_fw_size = fw->size;
	memcpy(constructed_fw, fw->data, fw->size);

	z2_setup(constructed_fw, constructed_fw_size, proxcal_fw, proxcal_fw_size, cal_fw, cal_fw_size);

	/* caller will call release_firmware */
}

static int iphone_multitouch_probe(struct platform_device *pdev)
{
	/* this driver is such a hack */
	if(multitouch_dev)
		return -EBUSY;

	multitouch_dev = &pdev->dev;

	printk("zephyr2: requesting Firmware\n");
	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG, "zephyr2.bin", multitouch_dev, NULL, got_constructed);
}

static int iphone_multitouch_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver iphone_multitouch_driver = {
	.probe = iphone_multitouch_probe,
	.remove = iphone_multitouch_remove,
	.suspend = NULL, /* optional but recommended */
	.resume = NULL,   /* optional but recommended */
	.driver = {
		.owner = THIS_MODULE,
		.name = "iphone-multitouch",
	},
};

static struct platform_device iphone_multitouch_dev = {
	.name = "iphone-multitouch",
	.id = -1,
};

static int __init iphone_multitouch_init(void)
{
	int ret;

	ret = platform_driver_register(&iphone_multitouch_driver);

	if (!ret) {
		ret = platform_device_register(&iphone_multitouch_dev);

		if (ret != 0) {
			platform_driver_unregister(&iphone_multitouch_driver);
		}
	}
	return ret;
}

static void __exit iphone_multitouch_exit(void)
{
	platform_device_unregister(&iphone_multitouch_dev);
	platform_driver_unregister(&iphone_multitouch_driver);
}

module_init(iphone_multitouch_init);
module_exit(iphone_multitouch_exit);

#include <asm/setup.h>
#define ATAG_IPHONE_PROX_CAL   0x54411004
#define ATAG_IPHONE_MT_CAL     0x54411005

struct atag_iphone_cal_data {
	u32 size;
	u8  data[];
};

static int __init parse_tag_prox_cal(const struct tag *tag)
{
	const struct atag_iphone_cal_data* cal_tag = (const struct atag_iphone_cal_data*)(((const u8*)tag) + sizeof(struct tag_header));

	proxcal_fw_size = cal_tag->size;
	memcpy(proxcal_fw, cal_tag->data, proxcal_fw_size);

	return 0;
}
__tagtable(ATAG_IPHONE_PROX_CAL, parse_tag_prox_cal);

static int __init parse_tag_mt_cal(const struct tag *tag)
{
	const struct atag_iphone_cal_data* cal_tag = (const struct atag_iphone_cal_data*)(((const u8*)tag) + sizeof(struct tag_header));

	cal_fw_size = cal_tag->size;
	memcpy(cal_fw, cal_tag->data, cal_fw_size);

	return 0;
}
__tagtable(ATAG_IPHONE_MT_CAL, parse_tag_mt_cal);

MODULE_DESCRIPTION("iPhone Zephyr 2 Multitouch Driver");
MODULE_AUTHOR("Yiduo Wang");
MODULE_LICENSE("GPL");
