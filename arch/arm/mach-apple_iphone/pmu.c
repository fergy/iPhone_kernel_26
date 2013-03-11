#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/i2c.h>
#include <mach/pmu.h>
#include <linux/i2c.h>
#include <mach/gpio.h>

/*typedef struct PMURegisterData {
	uint8_t reg;
	uint8_t data;
} PMURegisterData;*/


static struct i2c_client *pmu_i2c;
static struct rtc_device *rtc;



static int iphone_pmu_get_reg(int reg) {
	struct i2c_msg xfer[2];
	uint8_t out[1];
	int ret;
 
	xfer[0].addr = PMU_GETADDR;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = (u8 *)&reg;

	xfer[1].addr = PMU_GETADDR;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 1;
	xfer[1].buf = (u8 *)out;

	ret = i2c_transfer(pmu_i2c->adapter, xfer, 2);

	return out[0];
}

static int iphone_pmu_get_regs(int reg, uint8_t* out, int count) {
	struct i2c_msg xfer[2];
	int ret;
 
	xfer[0].addr = PMU_GETADDR;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = (u8 *)&reg;

	xfer[1].addr = PMU_GETADDR;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = count;
	xfer[1].buf = (u8 *)out;
 
	ret = i2c_transfer(pmu_i2c->adapter, xfer, 2);

	return ret;
}

static int iphone_pmu_write_reg(int reg, int data, int verify) {
	uint8_t command[2];
	uint8_t buffer = 0;

	struct i2c_msg xfer[2];

	command[0] = reg;
	command[1] = data;

	xfer[0].addr = PMU_SETADDR;
	xfer[0].flags = 0;
	xfer[0].len = sizeof(command);
	xfer[0].buf = (u8 *)command;

	i2c_transfer(pmu_i2c->adapter, xfer, 1);

	if(!verify)
		return 0;

	buffer = iphone_pmu_get_reg(reg);

	if(buffer == data)
		return 0;
	else
		return -1;
}

static int query_adc(int flags) {
	uint8_t lower;
	iphone_pmu_write_reg(PMU_ADCC3, 0, 0);
	iphone_pmu_write_reg(PMU_ADCC3, 0, 0);
	udelay(30);
	iphone_pmu_write_reg(PMU_ADCC2, 0, 0);
	iphone_pmu_write_reg(PMU_ADCC1, PMU_ADCC1_ADCSTART | (PMU_ADCC1_ADC_AV_16 << PMU_ADCC1_ADC_AV_SHIFT) | flags, 0);
	msleep(30);
	lower = iphone_pmu_get_reg(PMU_ADCS3);
	if((lower & 0x80) == 0x80) {
		uint8_t upper = iphone_pmu_get_reg(PMU_ADCS1);
		return ((upper << 2) | (lower & 0x3)) * 6000 / 1023;
	} else {
		return -1;
	}
}

static PowerSupplyType identify_usb_charger(void) {
	int dn;
	int dp;
	int x;

	iphone_gpio_pin_output(PMU_GPIO_CHARGER_IDENTIFY_DN, 1);
	dn = query_adc(PMU_ADCC1_ADCINMUX_ADCIN2_DIV << PMU_ADCC1_ADCINMUX_SHIFT);
	if(dn < 0)
		dn = 0;
	iphone_gpio_pin_output(PMU_GPIO_CHARGER_IDENTIFY_DN, 0);

	iphone_gpio_pin_output(PMU_GPIO_CHARGER_IDENTIFY_DP, 1);
	dp = query_adc(PMU_ADCC1_ADCINMUX_ADCIN2_DIV << PMU_ADCC1_ADCINMUX_SHIFT);
	if(dp < 0)
		dp = 0;
	iphone_gpio_pin_output(PMU_GPIO_CHARGER_IDENTIFY_DP, 0);

	if(dn < 99 || dp < 99) {
		return PowerSupplyTypeUSBHost;
	}

	x = (dn * 1000) / dp;
	if((x - 1291) <= 214) {
		return PowerSupplyTypeUSBBrick1000mA;
	}

	if((x - 901) <= 219 && dn <= 367 ) {
		return PowerSupplyTypeUSBBrick500mA;
	} else {
		return PowerSupplyTypeUSBHost;
	}
}

PowerSupplyType iphone_pmu_get_power_supply(void) {
	int mbcs1 = iphone_pmu_get_reg(PMU_MBCS1);

	if(mbcs1 & PMU_MBCS1_ADAPTPRES)
		return PowerSupplyTypeFirewire;

	if(mbcs1 & PMU_MBCS1_USBOK)
		return identify_usb_charger();
	else
		return PowerSupplyTypeBattery;

}

void iphone_pmu_charge_settings(int UseUSB, int SuspendUSB, int StopCharger)
{
	PowerSupplyType type = iphone_pmu_get_power_supply();

	if(type != PowerSupplyTypeUSBHost)	// No need to suspend USB, since we're not plugged into a USB host
		SuspendUSB = 0;

	if(SuspendUSB)
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_SUSPEND, 1);
	else
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_SUSPEND, 0);

	if(StopCharger) {
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_SUSPEND, 1);
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_SHUTDOWN, 1);
	} else {
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_SUSPEND, 0);
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_SHUTDOWN, 0);
	}

	if(type == PowerSupplyTypeUSBBrick500mA || type == PowerSupplyTypeUSBBrick1000mA || (type == PowerSupplyTypeUSBHost && UseUSB))
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_500_1000, 1);
	else
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_500_1000, 0);

	if(type == PowerSupplyTypeUSBBrick1000mA)
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_1000, 1);
	else
		iphone_gpio_pin_output(PMU_GPIO_CHARGER_USB_1000, 0);
}

int iphone_pmu_get_battery_voltage(void)
{
	return query_adc(PMU_ADCC1_ADCINMUX_BATSNS_DIV << PMU_ADCC1_ADCINMUX_SHIFT);
}

int iphone_pmu_write_regs(const PMURegisterData* regs, int num) {
	int i;
	for(i = 0; i < num; i++) {
		iphone_pmu_write_reg(regs[i].reg, regs[i].data, 1);
	}

	return 0;
}

static void iphone_pmu_write_oocshdwn(int data) {
	uint8_t registers[1];
	uint8_t discardData[5];
	uint8_t poweroffData[] = {7, 0xAA, 0xFC, 0x0, 0x0, 0x0};
	registers[0] = 2;

	i2c_master_send(pmu_i2c, registers, sizeof(registers));
	i2c_master_recv(pmu_i2c, discardData, sizeof(data));
	i2c_master_send(pmu_i2c, poweroffData, sizeof(poweroffData));
	//iphone_i2c_rx(PMU_I2C_BUS, PMU_GETADDR, registers, sizeof(registers), discardData, sizeof(data));
	//iphone_i2c_tx(PMU_I2C_BUS, PMU_SETADDR, poweroffData, sizeof(poweroffData));
	iphone_pmu_write_reg(PMU_OOCSHDWN, data, 0);
	while(1) {
		udelay(100000);
	}
}

static void iphone_pmu_poweroff(void) {
	//lcd_shutdown();
	iphone_pmu_write_oocshdwn(PMU_OOCSHDWN_GOSTBY);
}


/*static int iphone_pmu_get_dayofweek(void) {
	return iphone_pmu_get_reg(PMU_RTCWD) & PMU_RTCWD_MASK;
}

static int iphone_pmu_set_dayofweek(int num) {
	return iphone_pmu_write_reg(PMU_RTCWD, num & PMU_RTCWD_MASK, 0);
}*/

static unsigned long iphone_pmu_get_epoch(void)
{
	unsigned long secs;
	s32 offset;
	u8 rtc_data[PMU_RTCYR - PMU_RTCSC + 1];
	u8 rtc_data2[PMU_RTCYR - PMU_RTCSC + 1];

	do
	{
		iphone_pmu_get_regs(PMU_RTCSC, rtc_data, PMU_RTCYR - PMU_RTCSC + 1);
		iphone_pmu_get_regs(PMU_RTCSC, rtc_data2, PMU_RTCYR - PMU_RTCSC + 1);
	} while(rtc_data2[0] != rtc_data[0]);

	secs = mktime(
			2000 + bcd2bin(rtc_data[PMU_RTCYR - PMU_RTCSC] & PMU_RTCYR_MASK),
			rtc_data[PMU_RTCMT - PMU_RTCSC] & PMU_RTCMT_MASK,
			bcd2bin(rtc_data[PMU_RTCDT - PMU_RTCSC] & PMU_RTCDT_MASK),
			bcd2bin(rtc_data[PMU_RTCHR - PMU_RTCSC] & PMU_RTCHR_MASK),
			bcd2bin(rtc_data[PMU_RTCMN - PMU_RTCSC] & PMU_RTCMN_MASK),
			bcd2bin(rtc_data[0] & PMU_RTCSC_MASK)
			);

	iphone_pmu_get_regs(0x6B, (u8*) &offset, sizeof(offset));

	secs += offset;
	return secs;
}


static int iphone_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	rtc_time_to_tm(iphone_pmu_get_epoch(), rtc_tm);
	return 0;
}

static int iphone_rtc_settime(struct device *dev, struct rtc_time *rtc_tm)
{
	unsigned long secs;
	unsigned long cur;
	s32 offset;
	rtc_tm_to_time(rtc_tm, &secs);
	cur = iphone_pmu_get_epoch();
	offset = secs - cur;

	iphone_pmu_write_reg(0x6B, offset & 0xFF, 0);
	iphone_pmu_write_reg(0x6C, (offset >> 8) & 0xFF, 0);
	iphone_pmu_write_reg(0x6D, (offset >> 16) & 0xFF, 0);
	iphone_pmu_write_reg(0x6E, (offset >> 24) & 0xFF, 0);

	return 0;
}

static const struct rtc_class_ops iphone_rtcops = {
	.read_time	= iphone_rtc_gettime,
	.set_time	= iphone_rtc_settime,
};

static int pmu_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	int ret = 0;
	pmu_i2c = i2c;

	rtc = rtc_device_register("iphone", &i2c->dev, &iphone_rtcops,
			THIS_MODULE);

	if (IS_ERR(rtc)) {
		dev_err(&i2c->dev, "cannot attach rtc\n");
		ret = PTR_ERR(rtc);
		goto err_nortc;
	}

	return 0;

err_nortc:

	return ret;
}

static int __devexit pmu_i2c_remove(struct i2c_client *client)
{
	pmu_i2c = NULL;
	rtc_device_unregister(rtc);
	return 0;
}

static const struct i2c_device_id pmu_i2c_id[] = {
	{ "iphone-pmu", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pmu_i2c_id);

static struct i2c_driver pmu_i2c_driver = {
	.driver = {
		.name = "iphone-pmu",
		.owner = THIS_MODULE,
	},
	.probe = pmu_i2c_probe,
	.remove = pmu_i2c_remove,
	.id_table = pmu_i2c_id,
};

static int __init pmu_modinit(void)
{
	int ret;

	ret = i2c_add_driver(&pmu_i2c_driver);
	if (ret != 0)
		pr_err("pmu: Unable to register I2C driver: %d\n", ret);
	return ret;
}
module_init(pmu_modinit);

static void __exit pmu_exit(void)
{
	i2c_del_driver(&pmu_i2c_driver);
}
module_exit(pmu_exit);


MODULE_DESCRIPTION("iPhone pmu driver");
MODULE_AUTHOR("Fredrik Gustafsson <frgsutaf@kth.se>");
MODULE_LICENSE("GPL");

