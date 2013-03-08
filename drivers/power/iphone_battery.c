#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <mach/pmu.h>

static int iphone_battery_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val);

static int iphone_power_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val);

static char* supply_list[] =
{
	"battery",
};

static enum power_supply_property iphone_battery_properties[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
};

static enum power_supply_property iphone_power_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static struct power_supply battery =
{
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = iphone_battery_properties,
	.num_properties = ARRAY_SIZE(iphone_battery_properties),
	.get_property = iphone_battery_get_property,
};

static struct power_supply ac =
{
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = supply_list,
	.num_supplicants = ARRAY_SIZE(supply_list),
	.properties = iphone_power_properties,
	.num_properties = ARRAY_SIZE(iphone_power_properties),
	.get_property = iphone_power_get_property,
};

static struct power_supply usb =
{
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = supply_list,
	.num_supplicants = ARRAY_SIZE(supply_list),
	.properties = iphone_power_properties,
	.num_properties = ARRAY_SIZE(iphone_power_properties),
	.get_property = iphone_power_get_property,
};

struct iphone_battery_info {
	struct mutex lock;
	struct delayed_work monitor_work;

	int voltage;
	int level;
	PowerSupplyType type;
};

static struct iphone_battery_info iphone_batt_info;

static void iphone_battery_update_status(bool send_updates)
{
	int voltage;
	int level;
	PowerSupplyType type;
	PowerSupplyType old_type;
	bool changed = false;

	mutex_lock(&iphone_batt_info.lock);
	type = iphone_pmu_get_power_supply();
	voltage = iphone_pmu_get_battery_voltage();
	iphone_pmu_charge_settings(1, 0, 0);

	old_type = iphone_batt_info.type;
	
	if(iphone_batt_info.voltage != voltage)
		changed = true;
	
	level = ((voltage - 3500) * 100) / (4200 - 3500);
	if(level < 0)
		level = 0;
	if(level > 100)
		level = 100;

	iphone_batt_info.type = type;
	iphone_batt_info.voltage = voltage;
	iphone_batt_info.level = level;
	mutex_unlock(&iphone_batt_info.lock);

	if(send_updates)
	{
		if(type != old_type)
		{
			if(old_type == PowerSupplyTypeUSBHost)
				power_supply_changed(&usb);
			else if(old_type != PowerSupplyTypeBattery)
				power_supply_changed(&ac);

			if(type == PowerSupplyTypeUSBHost)
				power_supply_changed(&usb);
			else if(type != PowerSupplyTypeBattery)
				power_supply_changed(&ac);
		}

		if(changed)
			power_supply_changed(&battery);
	}
}

static void iphone_battery_work(struct work_struct* work)
{
	const int interval = msecs_to_jiffies(60 * 1000);
	iphone_battery_update_status(true);
	schedule_delayed_work(&iphone_batt_info.monitor_work, interval);
}

static int iphone_power_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	PowerSupplyType type;

	iphone_battery_update_status(false);

	mutex_lock(&iphone_batt_info.lock);
	type = iphone_batt_info.type;
	mutex_unlock(&iphone_batt_info.lock);

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			if (psy->type == POWER_SUPPLY_TYPE_MAINS) {
				val->intval = ((type ==  PowerSupplyTypeUSBBrick500mA || type == PowerSupplyTypeUSBBrick1000mA) ? 1 : 0);
			} else if (psy->type == POWER_SUPPLY_TYPE_USB) {
				val->intval = (type ==  PowerSupplyTypeUSBHost ? 1 : 0);
			} else
				val->intval = 0;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int iphone_battery_get_property(struct power_supply *psy,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	PowerSupplyType type;
	int voltage;
	int level;

	iphone_battery_update_status(false);

	mutex_lock(&iphone_batt_info.lock);
	type = iphone_batt_info.type;
	voltage = iphone_batt_info.voltage;
	level = iphone_batt_info.level;
	mutex_unlock(&iphone_batt_info.lock);

	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			if(type == PowerSupplyTypeBattery)
			{
			       val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
			} else
			{
				if(level == 100)
				       	val->intval = POWER_SUPPLY_STATUS_FULL;
				else
				       	val->intval = POWER_SUPPLY_STATUS_CHARGING;
			}
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			/* TODO: we might need to set POWER_SUPPLY_HEALTH_OVERHEAT if we figure out the battery temperature stuff */
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = 1;
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = level;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = voltage * 1000;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static int iphone_battery_probe(struct platform_device *pdev)
{
	int ret;

	ret = power_supply_register(&pdev->dev, &battery);
	if(ret)
		dev_err(&pdev->dev, "failed to register battery power supply!\n");

	ret = power_supply_register(&pdev->dev, &ac);
	if(ret)
		dev_err(&pdev->dev, "failed to register AC power supply!\n");

	ret = power_supply_register(&pdev->dev, &usb);
	if(ret)
		dev_err(&pdev->dev, "failed to register USB power supply!\n");

	iphone_battery_work(NULL);

	return 0;
}

static struct platform_driver iphone_battery_driver = {
	.probe  = iphone_battery_probe,
	.driver = {
		.name   = "iphone-battery",
		.owner  = THIS_MODULE,
	},
};

static struct platform_device iphone_battery_device = {
	.name = "iphone-battery",
	.id = -1,
};


static int __init iphone_battery_init(void)
{
	int ret;

	mutex_init(&iphone_batt_info.lock);
	INIT_DELAYED_WORK(&iphone_batt_info.monitor_work, iphone_battery_work);
	
	ret = platform_driver_register(&iphone_battery_driver);
	if (!ret) {
		ret = platform_device_register(&iphone_battery_device);

		if (ret != 0) {
			platform_driver_unregister(&iphone_battery_driver);
		}
	}

	return ret;
}

module_init(iphone_battery_init);
MODULE_DESCRIPTION("iPhone Battery Driver");
MODULE_LICENSE("GPL");
