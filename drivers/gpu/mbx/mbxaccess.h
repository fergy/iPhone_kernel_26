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

#ifndef __MBX_ACCESS_H__
#define __MBX_ACCESS_H__

#define __USER__

#define    LDM_2_6        1
#define    LDM_MVDPM      2

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#define    SUPPORT_LDM    LDM_2_6
#else
#if defined(MONTAVISTA_PM)
#define    SUPPORT_LDM    LDM_MVDPM
#endif
#endif

#define DRVNAME		MBXACCESS_DEVICE
#define DEVNAME		MBXACCESS_DEVICE


#define CLASSNAME   "powervr"

struct MBXAccess 
{
	struct fasync_struct *asyncqueue;
	int event;
};

typedef void (*SyncServiceHandler) (IMG_VOID *pvData);

#if defined(DEBUG)
typedef struct SyncServiceTag
{
	SyncServiceHandler	pfnHandler;
	char *psServiceName;
} SyncService;
#define SYNC_SERVICE_ENTRY(x) {x, #x}
#else
typedef SyncServiceHandler SyncService;
#define SYNC_SERVICE_ENTRY(x) x
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
#define STRUCT_DEVICE device
#else
#define STRUCT_DEVICE platform_device
#endif

static int __init  MBXAccess_Init(void);
static void __exit MBXAccess_Exit(void);

static int MBXAccess_Ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int MBXAccess_Open(struct inode *inode, struct file *filp);
static int MBXAccess_Release(struct inode *inode, struct file *filp);
static int MBXAccess_Mmap(struct file* pFile, struct vm_area_struct* ps_vma);
static int MBXAccess_FAsync(int fd, struct file *filp, int mode);

static void MBXCallSyncKernelServiceHandler(struct inode *inode, struct file *filp, unsigned long arg);

#if defined(SUPPORT_LDM) && defined(SUPPORT_POWER_MANAGEMENT)
#include <linux/device.h>

static int  MBXAccess_InitDrv(void);
static void MBXAccess_InitFailed(void);
static int  MBXAccess_DeInitDrv(void);
static int  MBXAccess_OpenComplete(void);
static int  MBXAccess_CloseComplete(void);

static void MBXSyncSrv_LoadDriverParams(IMG_VOID *pvData);


static int MBXAccess_DriverRemove(struct STRUCT_DEVICE *pDevice);
static int MBXAccess_DriverProbe(struct STRUCT_DEVICE *pDevice);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16))
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *pDevice, u32 level);
static int MBXAccess_DriverSuspend(struct STRUCT_DEVICE *pDevice, pm_message_t state, u32 level);
#else
static int MBXAccess_DriverSuspend(struct STRUCT_DEVICE *pDevice, pm_message_t state);
static int MBXAccess_DriverResume(struct STRUCT_DEVICE *pDevice);
#endif 
static void MBXAccess_DriverShutdown(struct STRUCT_DEVICE *pDevice);

#if defined(SUPPORT_LDM) && defined(SUPPORT_POWER_MANAGEMENT)
static void MBXAccess_DeviceRelease(struct device *device);
#endif 
#endif 
#endif 
