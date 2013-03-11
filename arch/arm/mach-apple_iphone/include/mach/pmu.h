#ifndef IPHONE_HW_PMU_H
#define IPHONE_HW_PMU_H

#include <linux/mfd/pcf50633/core.h>
#include <linux/mfd/pcf50633/pmic.h>
#include <linux/mfd/pcf50633/adc.h>
#include <linux/mfd/pcf50633/mbc.h>
#include <linux/power/ltc4088-charger.h>

extern struct pcf50633_platform_data pcf50633_pdata;

extern void pcf50633_power_off(void);
extern void pcf50633_suspend(void);
extern void ltc4088_set_charging_off(void);
extern void ltc4088_set_max_current(void);
extern struct ltc4088_charger_platform_data ltc4088_data;
void iphone_init_suspend(void);

#define PMU_I2C_BUS 0
#define PMU_SETADDR	0xE6
#define PMU_GETADDR	0xE7
#define PMU_MAXREG	0xF

#define PMU_GPIO_CHARGER_IDENTIFY_DN_IPHONE 0x1404
#define PMU_GPIO_CHARGER_IDENTIFY_DP_IPHONE 0x1405
#define PMU_GPIO_CHARGER_IDENTIFY_DN_IPOD 0x1603
#define PMU_GPIO_CHARGER_IDENTIFY_DP_IPOD 0x1604

#ifdef CONFIG_IPODTOUCH_1G
#define PMU_GPIO_CHARGER_IDENTIFY_DN PMU_GPIO_CHARGER_IDENTIFY_DN_IPOD
#define PMU_GPIO_CHARGER_IDENTIFY_DP PMU_GPIO_CHARGER_IDENTIFY_DN_IPOD
#else
#define PMU_GPIO_CHARGER_IDENTIFY_DN PMU_GPIO_CHARGER_IDENTIFY_DN_IPHONE
#define PMU_GPIO_CHARGER_IDENTIFY_DP PMU_GPIO_CHARGER_IDENTIFY_DN_IPHONE
#endif

#define PMU_GPIO_CHARGER_USB_SUSPEND 0x1704
#define PMU_GPIO_CHARGER_SUSPEND 0x1706
#define PMU_GPIO_CHARGER_SHUTDOWN 0x1707
#define PMU_GPIO_CHARGER_USB_500_1000 0x1705
#define PMU_GPIO_CHARGER_USB_1000 0x704

#define PMU_OOCSHDWN 0xC
#define PMU_GPIOCTL 0x13
#define PMU_GPIO1CFG 0x14
#define PMU_ADCC3 0x52
#define PMU_ADCC2 0x53
#define PMU_ADCC1 0x54
#define PMU_ADCS3 0x57
#define PMU_ADCS1 0x55
#define PMU_MBCS1 0x4B
#define PMU_RTCSC 0x59
#define PMU_RTCMN 0x5A
#define PMU_RTCHR 0x5B
#define PMU_RTCWD 0x5C
#define PMU_RTCDT 0x5D
#define PMU_RTCMT 0x5E
#define PMU_RTCYR 0x5F

#define PMU_RTCSC_MASK 0x7F //sec
#define PMU_RTCMN_MASK 0x7F //min
#define PMU_RTCHR_MASK 0x3F //hour
#define PMU_RTCWD_MASK 0x07 //wday
#define PMU_RTCDT_MASK 0x3F //mday
#define PMU_RTCMT_MASK 0x1F //mon
#define PMU_RTCYR_MASK 0xFF //year

#define PMU_OOCSHDWN_GOSTBY (1 << 0)

#define PMU_MBCS1_ADAPTPRES (1 << 2)
#define PMU_MBCS1_USBOK (1 << 0)

#define PMU_ADCC1_ADCINMUX_SHIFT 4
#define PMU_ADCC1_ADCINMUX_MASK 0xF
#define PMU_ADCC1_ADC_AV_SHIFT 2
#define PMU_ADCC1_ADCSTART 0x1
#define PMU_ADCC1_ADCRES 0x2

#define PMU_ADCC1_ADC_AV_1 0x0
#define PMU_ADCC1_ADC_AV_4 0x1
#define PMU_ADCC1_ADC_AV_8 0x2
#define PMU_ADCC1_ADC_AV_16 0x3

#define PMU_ADCC1_ADCINMUX_BATSNS_DIV 0x0
#define PMU_ADCC1_ADCINMUX_BATSNS_SUB 0x1
#define PMU_ADCC1_ADCINMUX_ADCIN2_DIV 0x2
#define PMU_ADCC1_ADCINMUX_ADCIN2_SUB 0x3
#define PMU_ADCC1_ADCINMUX_BATTEMP 0x6
#define PMU_ADCC1_ADCINMUX_ADCIN1 0x7

typedef struct PMURegisterData {
        uint8_t reg;
        uint8_t data;
} PMURegisterData;

typedef enum PowerSupplyType {
	PowerSupplyTypeError,
	PowerSupplyTypeBattery,
	PowerSupplyTypeFirewire,
	PowerSupplyTypeUSBHost,
	PowerSupplyTypeUSBBrick500mA,
	PowerSupplyTypeUSBBrick1000mA
} PowerSupplyType;

int iphone_pmu_get_battery_voltage(void);
void iphone_pmu_charge_settings(int UseUSB, int SuspendUSB, int StopCharger);
PowerSupplyType iphone_pmu_get_power_supply(void);
#endif

