/**********************************************************************
 *
 * Copyright(c) 2008 Imagination Technologies Ltd. All rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope it will be useful but, except 
 * as otherwise stated in writing, without any warranty; without even the 
 * implied warranty of merchantability or fitness for a particular purpose. 
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 * 
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * Imagination Technologies Ltd. <gpl-support@imgtec.com>
 * Home Park Estate, Kings Langley, Herts, WD4 8LZ, UK 
 *
 ******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>

#include <asm/uaccess.h>
#include <asm/signal.h>
#include <asm/siginfo.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
#include <linux/platform_device.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
typedef u32 pm_message_t;
#endif

#include "sysconfig.h"
#include "mbxaccessuser.h"
#include "services.h"
#include "mbxaccess_mbxsysdata.h"
#include "mbxaccess_mbxmem.h"
#include "pvr_debug.h"
#include "pvrmmap_private.h"
#include "mmap.h"
#include "mutex.h"

#include "mbxaccess_procwrapper.h"
#include "mbxaccess_iowrapper.h"
#include "mbxaccess_memwrapper.h"
#include "mbxaccess_virtmemwrapper.h"
#include "mbxaccess_timerwrapper.h"
#include "mbxaccess_mmapwrapper.h"
#include "mbxaccessinternals.h"
#include "mbxaccessapitype.h"
#include "mbxaccess.h"
#include "mbxworker_thread_struct.h"
#include "mbxaccess_virtmemwrapper_private.h"

#include "malloc_debug.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16))
#include <linux/clk.h>
#endif
#include <linux/delay.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15) && !defined(ARMVP))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33) && !defined(ARMVP))
#include <mach/clock.h>
#else
//#include <asm/arch/clock.h>
#include <mach/iphone-clock.h>
#endif
#endif


#define MBX_SERVICEAPI_NOTINITIALIZED	0

#include <linux/pm.h>

MODULE_AUTHOR("Imagination Technologies Ltd. <gpl-support@imgtec.com>");
MODULE_LICENSE("GPL");

PVRSRV_LINUX_MUTEX gPVRSRVLock;

module_init(MBXAccess_Init);
module_exit(MBXAccess_Exit);

struct timeval tv_before;
struct timeval tv_after;

static int   driverInitialised = 0;

int MBXMajor = 0;
int MBXMinor = 0;
int NumMBXs  = 1;

MBXDriverParams globalDriverParams;

IMG_CPU_VIRTADDR gpvLinBase = 0;
MBX_WORKER_THREAD_STRUCT *gpsInterruptStructKernel;

PVRSRV_LINUX_MUTEX MBX_Virtmem_lMallocHeadLock;

int gi32MBXInitialized = MBX_SERVICEAPI_NOTINITIALIZED;

static struct file_operations MBXAccess_fops =
{
	.owner   = THIS_MODULE,
	.ioctl   = MBXAccess_Ioctl,
	.open    = MBXAccess_Open,
	.release = MBXAccess_Release,
	.mmap    = MBXAccess_Mmap,
	.fasync  = MBXAccess_FAsync
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static struct resource device_resources[] =
{
	{
		.start = SOC_REG_BASE,
		.end = SOC_REG_BASE + SOC_REG_RANGE - 1,
		.flags = IORESOURCE_MEM
	},
	{
		.start = MBX_REG_SYS_PHYS_BASE,
		.end = MBX_REG_SYS_PHYS_BASE + MBX_REG_RANGE - 1,
		.flags = IORESOURCE_MEM
	},
	{
		.start = MBX_SP_3D_SYS_PHYS_BASE,
		.end = MBX_SP_3D_SYS_PHYS_BASE + MBX_SP_3D_RANGE - 1,
		.flags = IORESOURCE_MEM
	},
	{
		.start = MBX_SP_2D_SYS_PHYS_BASE,
		.end = MBX_SP_2D_SYS_PHYS_BASE + MBX_SP_2D_RANGE - 1,
		.flags = IORESOURCE_MEM
	},
	{
		.start = MBX_SP_TA_CTRL_SYS_PHYS_BASE,
		.end = MBX_SP_TA_CTRL_SYS_PHYS_BASE + MBX_SP_TA_CTRL_RANGE - 1,
		.flags = IORESOURCE_MEM
	}
};
#else
struct device_class powervr_devclass = {
    .name       = "powervr",
};

static ssize_t MBXAccess_ShowDev(struct device *pDevice, char *buf, size_t count, loff_t off)
{
	ssize_t ret = off ? 0 : snprintf(&buf[off], count, "%d:0\n", MBXMajor);
	return ret;
}

static DEVICE_ATTR(dev, S_IRUGO, MBXAccess_ShowDev, NULL);

static int MBXAccess_DriverScale(struct bus_op_point *op, u32 level)
{
	return 0;
}
#endif 



unsigned long lastFAsyncTime = 0;

static int PerDeviceSysInitialise(IMG_PVOID pHostOSDevice);
static int PerDeviceSysDeInitialise(IMG_PVOID pHostOSDevice);

static int PerDeviceSysInitialise(IMG_PVOID pHostOSDevice)
{
#if defined(OMAP2430)
#if defined(DEBUG) || defined(TIMING)
	struct device * pDevice = (struct device *)pHostOSDevice;
	struct clk *pGpt12_fck;
	struct clk *pGpt12_ick;

	
	pGpt12_fck = clk_get(pDevice, "gpt12_fck");
	pGpt12_ick = clk_get(pDevice, "gpt12_ick");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	clk_enable(pGpt12_fck);
	clk_enable(pGpt12_ick);
#else
	clk_use(pGpt12_fck);
	clk_use(pGpt12_ick);
#endif 
#else
	PVR_UNREFERENCED_PARAMETER(pHostOSDevice);
#endif 
#else 
	PVR_UNREFERENCED_PARAMETER(pHostOSDevice);
#endif 
	return 0;
}

static int PerDeviceSysDeInitialise(IMG_PVOID pHostOSDevice)
{
#if defined(OMAP2430)
#if defined(DEBUG) || defined(TIMING)
	struct device * pDevice = (struct device *)pHostOSDevice;
	struct clk *pGpt12_fck;
	struct clk *pGpt12_ick;

	
	pGpt12_fck = clk_get(pDevice, "gpt12_fck");
	pGpt12_ick = clk_get(pDevice, "gpt12_ick");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
	clk_disable(pGpt12_fck);
	clk_put(pGpt12_fck);
	
	clk_disable(pGpt12_ick);
	clk_put(pGpt12_ick);
#else
	clk_unuse(pGpt12_fck);
	clk_unuse(pGpt12_ick);
#endif 
#else
	PVR_UNREFERENCED_PARAMETER(pHostOSDevice);
#endif 
#else
	PVR_UNREFERENCED_PARAMETER(pHostOSDevice);
#endif 
	return 0;
}

struct MBXAccess device;

static void MBXAccess_DeviceRelease(struct device *pDevice);
static int MBXAccess_DriverProbe(struct STRUCT_DEVICE *pDevice);
static int MBXAccess_DriverRemove(struct STRUCT_DEVICE *pDevice);
static int MBXAccess_DriverSuspend(struct STRUCT_DEVICE *device, pm_message_t state
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
								   , u32 level
#endif
								  );
static void MBXAccess_DriverShutdown(struct STRUCT_DEVICE *pDevice);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *device, u32 level);
#else
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *device);
#endif



static struct platform_device powervr_device =
{
	.name           = DEVNAME,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	.id             = -1,
	.num_resources  = (sizeof (device_resources) / sizeof (struct resource)),
	.resource       = device_resources,
#else 
	.id             = 0,
#endif 
	.dev            =
	{
	   .release    = MBXAccess_DeviceRelease
	}
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
static struct platform_driver powervr_driver =
{
	.driver = {
		.name		= DEVNAME,
		.owner		= THIS_MODULE,
		.bus		= &platform_bus_type,
	},
	.probe      = MBXAccess_DriverProbe,
	.remove     = MBXAccess_DriverRemove,
	.suspend    = MBXAccess_DriverSuspend,
	.resume     = MBXAccess_DriverResume,
	.shutdown   = MBXAccess_DriverShutdown,
};
#else
static struct device_driver powervr_driver =
{
	.name       = DEVNAME,
	.bus        = &platform_bus_type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	.devclass	= &powervr_devclass,
	.scale		= MBXAccess_DriverScale,
#endif
	.probe      = MBXAccess_DriverProbe,
	.remove     = MBXAccess_DriverRemove,
	.suspend    = MBXAccess_DriverSuspend,
	.resume     = MBXAccess_DriverResume,
	.shutdown   = MBXAccess_DriverShutdown,
};
#endif

static int MBXAccess_InitDrv(void);
static int MBXAccess_DeInitDrv(void);
static int MBXAccess_OpenComplete(void);
static int MBXAccess_CloseComplete(void);
static void MBXAccess_InitFailed(void);



static int MBXAccess_Ioctl(struct inode *inode, struct file *filp,
				 unsigned int cmd, unsigned long arg)
{
	int err = 0;

	PVR_DPF((PVR_DBG_MESSAGE, "Requesting service cmd=%x", cmd));
	LinuxLockMutex(&gPVRSRVLock);
	
	switch (cmd)
	{
		case MBXCallSyncKernelService:
		{
			PVR_DPF((PVR_DBG_MESSAGE, "Requesting SyncKernelService"));
			
			MBXCallSyncKernelServiceHandler(inode, filp, arg);
		}
		break;
		case MBXInitDrv:
		{
			err = MBXAccess_InitDrv();
		}
		break;
		case MBXDeInitDrv:
		{
			err = MBXAccess_DeInitDrv();
		}
		break;
		case MBXRecyleDriver:
		{
			if(driverInitialised)
			{
				MBXAccess_DeInitDrv();
			}
			PVRMMapCleanup();
		}
		break;
		case MBXFailed:
		{
			MBXAccess_InitFailed();
		}
		break;
		case MBXOpenComplete:
		{
			err = MBXAccess_OpenComplete();
		}
		break;
		case MBXCloseComplete:
		{
			err = MBXAccess_CloseComplete();
		}
		break;
		default:
			;
	};

	LinuxUnLockMutex(&gPVRSRVLock);
	return err;
}

static int MBXAccess_Open(struct inode *inode, struct file *filp)
{
	int err = 0;

	PVR_UNREFERENCED_PARAMETER(inode);
	PVR_UNREFERENCED_PARAMETER(filp);

	globalDriverParams.MBXDriverParam_PVRSRV_DEVICE_NODE_SIZE = 0;
	globalDriverParams.MBXDriverParam_SYS_DATA_SIZE = 0;
	globalDriverParams.debugLevel = 0;

	filp->private_data = &device;

	return err;
}

void MBXAccess_KillFAsync(void)
{
	lastFAsyncTime = jiffies;
	kill_fasync(&device.asyncqueue, SIGIO, POLL_IN);
}

void MBXAccess_KillFAsyncVSync(void)
{
	lastFAsyncTime = jiffies;
	kill_fasync(&device.asyncqueue, SIGIO, POLL_IN);
}

void MBXAccess_KillFAsyncLimited(unsigned long timebreak)
{
	if (jiffies - lastFAsyncTime > timebreak)
	{
		MBXAccess_KillFAsync();
	}
}

static int MBXAccess_FAsync(int fd, struct file *filp, int mode)
{
	struct MBXAccess* dev = filp->private_data;
	return fasync_helper(fd, filp, mode, &dev->asyncqueue);
}

static int MBXAccess_Release(struct inode *inode, struct file *filp)
{
	int err = 0;
	
	PVR_UNREFERENCED_PARAMETER(inode);
	PVR_UNREFERENCED_PARAMETER(filp);
	
	if(filp->f_flags & FASYNC)
	{
		
		MBXAccess_FAsync(-1, filp, 0);
	}
	return err;
}

static int MBXAccess_Mmap(struct file* pFile, struct vm_area_struct* ps_vma)
{
	int err = 0;
	err = PVRMMap(pFile, ps_vma);
	return err;
}


static int MBXAccess_DriverProbe(struct STRUCT_DEVICE *pDevice)
{
	int ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	device_create_file(pDevice, &dev_attr_dev);
#endif
	if (PerDeviceSysInitialise((IMG_PVOID)pDevice) != 0)
	{
		ret = -EINVAL;
		goto err_out;
	}
	else
	{
		ret = 0;
	}

	return ret;
err_out:
	return ret;
}

static int MBXAccess_DriverRemove(struct STRUCT_DEVICE *pDevice)
{
	int ret = 0;

	if (PerDeviceSysDeInitialise((IMG_PVOID)pDevice) != 0)
	{
		ret = -EINVAL;
	}
	else
	{
		ret = 0;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	device_remove_file(pDevice, &dev_attr_dev);
#endif

	return ret;
}

static int MBXAccess_DriverSuspend(struct STRUCT_DEVICE *device, pm_message_t state
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
								   , u32 level
#endif
								  )
{
	
	return 0;
}

static void MBXAccess_DriverShutdown(struct STRUCT_DEVICE *pDevice)
{
	PVR_UNREFERENCED_PARAMETER(pDevice);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *device, u32 level)
#else
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *device)
#endif
{
	
	return 0;
}

static void MBXAccess_DeviceRelease(struct device *pDevice)
{
	PVR_UNREFERENCED_PARAMETER(pDevice);
}

static void MBXAccess_InitFailed(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
	unregister_chrdev(MBXMajor, DRVNAME);
#else
	if(!unregister_chrdev(MBXMajor, DRVNAME))
#endif
	{
		MBXMajor = 0;
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16))
#define SUCCESSFUL(x) if(x) { err = -ENOMEM; goto init_exit; }
#else
#define SUCCESSFUL(x)
#endif
static int MBXAccess_InitDrv(void)
{
	int err = 0;
	IMG_CPU_PHYADDR sRegsCPUPhysBase = {MBX_REG_SYS_PHYS_BASE};

	
	gpvLinBase = HostMapPhysToLin (sRegsCPUPhysBase, MBX_REG_RANGE, CACHETYPE_UNCACHED | EXTRA_CACHETYPE_SHARED);

	driverInitialised = 1;
	return err;
}

static int MBXAccess_DeInitDrv(void)
{
	int err = 0;

	driverInitialised = 0;

	if(gpvLinBase)
	{
		
	}
	return err;
}


static int __init MBXAccess_Init(void)
{
	int err = PVRSRV_OK;
	int i = 0;

	PVRMMapInit();

#if defined(DEBUG) && defined(FULL_DEBUG)
	PVRDebugSetLevel(DBGPRIV_ALLLEVELS);
#endif

	PVR_DPF((PVR_DBG_MESSAGE,"Module load starts"));

	MALLOC_DEBUG_INIT();
	LinuxInitMutex(&gPVRSRVLock);
	LinuxInitMutex(&MBX_Virtmem_lMallocHeadLock);

	gpsMBXDeviceList = (MBX_DEVICELIST*)KMALLOC(sizeof(MBX_DEVICELIST), GFP_KERNEL);
	gpsMBXDeviceList->psDeviceNodeList = IMG_NULL;
	for(i = 0; i < SYS_DEVICE_COUNT; i++)
	{
		gpsMBXDeviceList->sDeviceID[i].uiID = i;
		gpsMBXDeviceList->sDeviceID[i].bInUse = IMG_FALSE;
	}
	gpsMBXDeviceList->ui32NumDevices = SYS_DEVICE_COUNT;

	MBXMajor = register_chrdev(0, DRVNAME, &MBXAccess_fops);
	if (MBXMajor <= 0)
	{
		PVR_DPF((PVR_DBG_ERROR,"MBX dev number allocation failure - %d", MBXMajor));
		err = -EBUSY;
	}
	else
	{
		PVR_DPF((PVR_DBG_MESSAGE,"MBX device number: %d", MBXMajor));
		err = 0;
	}
	PVR_DPF((PVR_DBG_MESSAGE,"Module load ends"));

	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	err = devclass_register(&powervr_devclass);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
	SUCCESSFUL(platform_driver_register(&powervr_driver));
#else 
	driver_register(&powervr_driver);
#endif 

#if defined(TIMING)
	if ((err = platform_device_register(&powervr_device)) != 0)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
		platform_driver_unregister(&powervr_driver);
#else
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
		devclass_unregister(&powervr_devclass);
#endif
		driver_unregister(&powervr_driver);
#endif
		PVR_DPF((PVR_DBG_ERROR, "Unable to create platform device err = %u", err));
		return err;
	}
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16))
init_exit:
#endif
	return err;
}

static void __exit MBXAccess_Exit(void)
{
	if(driverInitialised)
		MBXAccess_DeInitDrv();

#if defined(TIMING)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
	platform_device_unregister(&powervr_device);
#endif
#endif 

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))	
	platform_driver_unregister(&powervr_driver);
#else
	driver_unregister(&powervr_driver);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	devclass_unregister(&powervr_devclass);
#endif
	
	PVRMMapCleanup();

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
	unregister_chrdev(MBXMajor, DRVNAME);
#else
	if (unregister_chrdev(MBXMajor, DRVNAME))
	{
		PVR_DPF((PVR_DBG_FATAL,"Unable to unregister device %d", MBXMajor));
	}
#endif
	KFREE(gpsMBXDeviceList);
	MALLOC_DEBUG_EXIT();
}


void MBXSyncSrv_GetServiceAPIInitStatus(IMG_VOID *pvData)
{
	int status = 0;
	
	status = gi32MBXInitialized;
	
	COPY_TO_USER(pvData,(void*)&status, sizeof(int));
}

void MBXSyncSrv_SetServiceAPIInitStatus(IMG_VOID *pvData)
{
	int infoFromUser = 0;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(int));
	gi32MBXInitialized = infoFromUser;
}
static void MBXSyncSrv_LoadDriverParams(IMG_VOID *pvData);

SyncService syncServiceTable[MBXKERNELSYNCSERVICE_END] =
{
	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_RequestIrqHandler),

	 SYNC_SERVICE_ENTRY(MBXSyncSrv_FreeIrqHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostMapPhysToLinHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_PCIFindSlotHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostPCIReadDwordHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_PageToPhysHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_VirtualAllocateReserveHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_VirtualDeallocateUnreserveHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_ConvertKVToPageHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostFlushCpuCacheAreaHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_PVRSRVFlushCpuCacheKMHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_CreateProcEntriesHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_RemoveProcEntriesHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostAddTimerHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostRemoveTimerHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_MMapGetFullMapDataHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostReadHWRegHandler),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_HostWriteHWRegHandler),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_LoadDriverParams),

	
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_DeviceNodeCreate),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_DeviceNodeDestroy),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_GetDeviceNodeValue),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_SetDeviceNodeValue),

	 SYNC_SERVICE_ENTRY(MBXSyncSrv_GetServiceAPIInitStatus),
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_SetServiceAPIInitStatus),
#if defined(DEBUG)
	 SYNC_SERVICE_ENTRY(MBXSyncSrv_PrintLineIdentifier)
#endif
};

static void MBXCallSyncKernelServiceHandler(struct inode *inode, struct file *filp, unsigned long arg)
{
	MBXInfoForKernelSyncService infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, (void*)(arg), sizeof(MBXInfoForKernelSyncService));
#if defined(DEBUG)	
	PVR_DPF((PVR_DBG_MESSAGE, "Requesting %s", syncServiceTable[infoFromUser.id].psServiceName ));
	syncServiceTable[infoFromUser.id].pfnHandler((IMG_VOID*)infoFromUser.data);
#else
	syncServiceTable[infoFromUser.id]((IMG_VOID*)infoFromUser.data);
#endif
}

static int MBXAccess_OpenComplete(void)
{
	return 0;
}

static int MBXAccess_CloseComplete(void)
{
	return 0;
}

static void MBXSyncSrv_LoadDriverParams(IMG_VOID *pvData)
{
	COPY_FROM_USER((void*)&globalDriverParams, pvData, sizeof(MBXDriverParams));
}
