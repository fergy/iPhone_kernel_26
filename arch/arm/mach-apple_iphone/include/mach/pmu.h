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

#endif

