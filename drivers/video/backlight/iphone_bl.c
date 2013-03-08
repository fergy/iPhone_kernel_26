/*
 *  Backlight Driver for the iPhone Backlight
 *
 *  Copyright (c) 2010 Patrick Wildt
 *
 *  Based on kb6886_bl.c by Claudio Nieder
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <mach/pmu.h>

#define LCD_MAX_BACKLIGHT 45
#define LCD_BACKLIGHT_REG 0x28
#define LCD_BACKLIGHT_REGMASK 0x3F

static const PMURegisterData backlightOffData = {0x29, 0x0};

static const PMURegisterData backlightData[] = {
        {0x17, 0x1},
        {0x2A, 0x0},
        {0x28, 0x22},
        {0x29, 0x1},
        {0x2A, 0x6}
};

static void iphone_bl_set_intensity(int level)
{
        if(level == 0) {
                iphone_pmu_write_regs(&backlightOffData, 1);
        } else {
                PMURegisterData myBacklightData[sizeof(backlightData)/sizeof(PMURegisterData)];

                memcpy(myBacklightData, backlightData, sizeof(myBacklightData));

                if(level <= LCD_MAX_BACKLIGHT) {
                        int i;
                        for(i = 0; i < (sizeof(myBacklightData)/sizeof(PMURegisterData)); i++) {
                                if(myBacklightData[i].reg == LCD_BACKLIGHT_REG) {
                                        myBacklightData[i].data = level & LCD_BACKLIGHT_REGMASK;
                                }
                        }
                }
                iphone_pmu_write_regs(myBacklightData, sizeof(myBacklightData)/sizeof(PMURegisterData));
        }
}

struct iphonebl_machinfo {
	int max_intensity;
	int default_intensity;
	int limit_mask;
	void (*set_bl_intensity)(int intensity);
};

static struct iphonebl_machinfo iphone_bl_machinfo = {
	.max_intensity = 0x2d,
	.default_intensity = 0x14,
//	.limit_mask = 0x7f,
	.set_bl_intensity = iphone_bl_set_intensity,
};

static struct platform_device iphonebl_device = {
	.name		= "iphone-bl",
	.dev		= {
		.platform_data	= &iphone_bl_machinfo,
	},
	.id		= -1,
};

static struct platform_device *devices[] __initdata = {
	&iphonebl_device,
};

/*
 * Back to driver
 */

static int iphonebl_intensity;
static struct backlight_device *iphone_backlight_device;
static struct iphonebl_machinfo *bl_machinfo;

static int iphonebl_send_intensity(struct backlight_device *bd)
{
	int intensity = bd->props.brightness;

	if (bd->props.power != FB_BLANK_UNBLANK)
		intensity = 0;
	if (bd->props.fb_blank != FB_BLANK_UNBLANK)
		intensity = 0;

	bl_machinfo->set_bl_intensity(intensity);

	iphonebl_intensity = intensity;
	return 0;
}

static int iphonebl_get_intensity(struct backlight_device *bd)
{
	return iphonebl_intensity;
}

static struct backlight_ops iphonebl_ops = {
	.get_brightness = iphonebl_get_intensity,
	.update_status  = iphonebl_send_intensity,
};

static int iphonebl_probe(struct platform_device *pdev)
{
	struct iphonebl_machinfo *machinfo = pdev->dev.platform_data;

	bl_machinfo = machinfo;
	if (!machinfo->limit_mask)
		machinfo->limit_mask = -1;

	iphone_backlight_device = backlight_device_register("iphone-bl",
		&pdev->dev, NULL, &iphonebl_ops);
	if (IS_ERR(iphone_backlight_device))
		return PTR_ERR(iphone_backlight_device);

	platform_set_drvdata(pdev, iphone_backlight_device);

	iphone_backlight_device->props.max_brightness = machinfo->max_intensity;
	iphone_backlight_device->props.power = FB_BLANK_UNBLANK;
	iphone_backlight_device->props.brightness = machinfo->default_intensity;
	backlight_update_status(iphone_backlight_device);

	return 0;
}

static int iphonebl_remove(struct platform_device *pdev)
{
	struct backlight_device *bd = platform_get_drvdata(pdev);

	backlight_device_unregister(bd);

	return 0;
}

static struct platform_driver iphonebl_driver = {
	.probe		= iphonebl_probe,
	.remove		= iphonebl_remove,
	.driver		= {
		.name	= "iphone-bl",
	},
};

static int __init iphone_init(void)
{
	platform_add_devices(devices, ARRAY_SIZE(devices));
	return platform_driver_register(&iphonebl_driver);
}

static void __exit iphone_exit(void)
{
	platform_driver_unregister(&iphonebl_driver);
}

module_init(iphone_init);
module_exit(iphone_exit);

MODULE_AUTHOR("Patrick Wildt <webmaster@patrick-wildt.de");
MODULE_DESCRIPTION("iPhone Backlight Driver");
MODULE_LICENSE("GPL");
