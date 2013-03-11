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

#ifndef AUTOCONF_INCLUDED
#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
   #include <linux/modversions.h>
   #define MODVERSIONS
#endif
#endif

#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <asm/io.h>

#include "services.h"
#include "mbxaccess_mbxsysdata.h"
#include "mbxworker_thread_struct.h"
#include "pvrmmap_private.h"
#include "mbxaccess_iowrappertype.h"
#include "malloc_debug.h"
#include "hostfunc.h"
#include "mbxaccess_virtmemwrapper_private.h"
#include "mmap.h"

#include "pvr_debug.h"
#include "mbx1defs.h"

#warning HACK: pci_get_slot with incorrect parameters. PCI will crash anyway on iPhone 2.6.33..
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
#define pci_get_slot pci_find_slot
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#define irqreturn_t void
#define IRQ_HANDLED
#endif
typedef irqreturn_t (*RequestIrqReturnHandler)(int, void*, struct pt_regs *);
#else
typedef irqreturn_t (*RequestIrqReturnHandler)(int, void*);
#endif

static IMG_BOOL bMBXIsrInstalled = IMG_FALSE;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
static irqreturn_t deviceisr_wrapperInKS(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t deviceisr_wrapperInKS(int irq, void *dev_id);
#endif

extern IMG_CPU_VIRTADDR gpvLinBase;

extern IMG_BOOL gbDeviceIRQEnabled;

extern void MBXAccess_KillFAsync(void);
extern void MBXAccess_KillFAsyncVSync(void);

#define CACHETYPE_UNCACHED          0x00000001 
#define CACHETYPE_CACHED            0x00000002 
#define CACHETYPE_WRITECOMBINED     0x00000004 

#define SYS_KICKER_VALUE MBX1_INT_ISP

extern MBXDriverParams globalDriverParams;
extern MBX_WORKER_THREAD_STRUCT *gpsInterruptStructKernel;

char *IRQdevname = 0;

void MBXSyncSrv_RequestIrqHandler(IMG_VOID *pvData)
{
	MBXUserDataForRequestIrq infoFromUser;

	if(!bMBXIsrInstalled)
	{
		COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForRequestIrq));

		IRQdevname = KMALLOC(infoFromUser.size,GFP_KERNEL);

		COPY_FROM_USER((void*)IRQdevname, (void*)(infoFromUser.devname), infoFromUser.size);

		PVR_DPF((PVR_DBG_VERBOSE, "request_irq at %d", infoFromUser.irq));

		infoFromUser.ret = request_irq(infoFromUser.irq, deviceisr_wrapperInKS,
		#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
				SA_SHIRQ,
		#else
				IRQF_SHARED,
		#endif
			IRQdevname, infoFromUser.dev_id);
		if(infoFromUser.ret)
		{
			PVR_DPF((PVR_DBG_ERROR, "MBX IRQ request failed!"));
		}
		else
		{
			bMBXIsrInstalled = IMG_TRUE;
		}
	}
	else
	{
		PVR_DPF((PVR_DBG_WARNING, "Isr is already installed."));
	}
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForRequestIrq));
}

void MBXSyncSrv_FreeIrqHandler(IMG_VOID *pvData)
{
	MBXUserDataForFreeIrq infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForFreeIrq));

	free_irq(infoFromUser.irq, (void*)(infoFromUser.data));
	bMBXIsrInstalled = IMG_FALSE;
	if (IRQdevname)
	{
		 KFREE(IRQdevname);
	}
	IRQdevname = 0;
}

void MBXSyncSrv_HostMapPhysToLinHandler(IMG_VOID *pvData)
{
	MBXUserDataForHostMapPhysToLin infoFromUser;
	
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostMapPhysToLin));
	infoFromUser.returnedIOMem = HostMapPhysToLin(infoFromUser.offset, infoFromUser.size,infoFromUser.cacheType);

	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForHostMapPhysToLin));
}

void MBXSyncSrv_PCIFindSlotHandler(IMG_VOID *pvData)
{
	MBXUserDataForPCIFindSlot infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForPCIFindSlot));

//	infoFromUser.retPCIDevPointerInKS = pci_get_slot(infoFromUser.bus, infoFromUser.devfn);
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForPCIFindSlot));
}

void MBXSyncSrv_HostPCIReadDwordHandler(IMG_VOID *pvData)
{
	struct pci_dev *dev;

	MBXUserDataForHostPCIReadWriteDword infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostPCIReadWriteDword));

//	dev = pci_get_slot(infoFromUser.bus, PCI_DEVFN(infoFromUser.devfn, infoFromUser.func));

//	if (dev)
//	{
//		pci_read_config_dword(dev, (int)infoFromUser.reg, (u32*)(infoFromUser.value));
//	}
//	else
//	{
		infoFromUser.value = 0;
//	}
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForHostPCIReadWriteDword));
}

void MBXSyncSrv_HostPCIReadWriteDwordHandler(IMG_VOID *pvData)
{
	struct pci_dev *dev;

	MBXUserDataForHostPCIReadWriteDword infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostPCIReadWriteDword));

/*	dev = pci_get_slot(infoFromUser.bus, PCI_DEVFN(infoFromUser.devfn, infoFromUser.func));

	if (dev)
	{
		pci_write_config_dword(dev, (int) infoFromUser.reg, infoFromUser.value);
	}*/
}

IMG_CPU_VIRTADDR
HostMapPhysToLin (IMG_CPU_PHYADDR BasePAddr, IMG_UINT32 ui32Bytes, IMG_UINT32 ui32CacheType)
{
	IMG_VOID	*pvLin = IMG_NULL;
	IMG_UINT32	ui32Flags = 0;

	
	PMEM_AREA		psMemArea;
	PVR_DPF((PVR_DBG_MESSAGE, "HostMapPhysToLin: %x %x %x", BasePAddr, ui32Bytes, ui32CacheType));
	
	if(ui32CacheType & CACHETYPE_WRITECOMBINED)
	{
		ui32Flags = PVRSRV_HAP_WRITECOMBINE;
	}
	else
	{
		if(ui32CacheType & CACHETYPE_UNCACHED)
		{
			ui32Flags = PVRSRV_HAP_UNCACHED;
		}
		else
		{
			ui32Flags = PVRSRV_HAP_CACHED;
		}
	}
	psMemArea = MEM_AREA_NewIORemap(BasePAddr, ui32Bytes, ui32Flags);
	if(!PVRMMapRegisterArea("Physical", psMemArea, ui32Flags))
	{
		PVR_DPF((PVR_DBG_ERROR, "Unable to register area"));
		MEM_AREA_AreaDeepFree(psMemArea);
		return NULL;
	}
	pvLin = MEM_AREA_ToCpuVAddr(psMemArea);
	return pvLin;
}

static INLINE
IMG_UINT32 MBXDisableInterrupts (IMG_PVOID  pvLinRegBaseAddr,
								 IMG_UINT32 ui32Interrupts,
								 IMG_BOOL   bInterruptSense)
{
	IMG_UINT32 uOrigVal;

	
	uOrigVal = ReadHWReg(pvLinRegBaseAddr, MBX1_GLOBREG_INT_MASK);

	if(bInterruptSense)
	{
		WriteHWReg(pvLinRegBaseAddr, MBX1_GLOBREG_INT_MASK, uOrigVal & ~ui32Interrupts);
		return uOrigVal;
	}
	else
	{
		WriteHWReg(pvLinRegBaseAddr, MBX1_GLOBREG_INT_MASK, uOrigVal | ui32Interrupts);
		return ~uOrigVal;
	}
}

IMG_VOID SetDataOnQueueHead(IMG_VOID)
{
	gpsInterruptStructKernel->bDataOnQueue = IMG_TRUE;

	
	MBXAccess_KillFAsync();
}

IMG_VOID VSyncCommandComplete(IMG_HANDLE handle)
{
	if (gpsInterruptStructKernel->bVSyncCmdComplete)
	{
		PVR_DPF((PVR_DBG_ERROR, "VSYNC error\n"));
	}
	gpsInterruptStructKernel->bVSyncCmdComplete = IMG_TRUE;
	gpsInterruptStructKernel->pVSyncData = handle;
	
	MBXAccess_KillFAsyncVSync();
}

static 
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
irqreturn_t deviceisr_wrapperInKS(int irq, void *dev_id, struct pt_regs *regs)
#else
irqreturn_t deviceisr_wrapperInKS(int irq, void *dev_id)
#endif
{
	
	PVRSRV_DEVICE_NODE *psDeviceNode = (PVRSRV_DEVICE_NODE *)dev_id;

	if(!gpvLinBase)
	{
		return IRQ_HANDLED;
	}
	
	if (psDeviceNode && psDeviceNode->pfnDeviceISR)
	{
		
		IMG_UINT32 ui32MBXIntEnable = MBXDisableInterrupts (gpvLinBase, MBX1_INT_ALL, gpsInterruptStructKernel->bInterruptSense);
		
		IMG_UINT32 ui32MBXIntStatus = ReadHWReg(gpvLinBase, MBX1_GLOBREG_INT_STATUS);

		gpsInterruptStructKernel->ui32InterruptStatus = ui32MBXIntStatus & ui32MBXIntEnable;

		gpsInterruptStructKernel->pvDeviceInfo = psDeviceNode->pvISRData;
		gpsInterruptStructKernel->pfnDeviceISR = psDeviceNode->pfnDeviceISR;

		
		gpsInterruptStructKernel->ui32MBXIntEnable = ui32MBXIntEnable;

		
		MBXAccess_KillFAsync();
	}
	return IRQ_HANDLED;
}

void MBXSyncSrv_PageToPhysHandler(IMG_VOID *pvData)
{
	MBXUserDataForPageToPhys infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForPageToPhys));
	
	infoFromUser.uiAddr = (int)page_to_phys(((struct page*)infoFromUser.page));

	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForPageToPhys));
}
