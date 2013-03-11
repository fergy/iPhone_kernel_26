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

#ifndef _SYSCOMMON_H_
#define _SYSCOMMON_H_

#include "sysconfig.h"		
#include "sysinfo.h"		
#if defined(UBUILD)
#include "queue.h"
#include "power.h"
#include "resman.h"
#endif
#include "device.h"
#include "buffer_manager.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define SYS_MMU_NONE		0
#define SYS_MMU_LINEAR		1
#define SYS_MMU_NORMAL		2
typedef int SYS_MMU_MODE;

typedef enum _DEV_CLOCKGATE_CORE{
	DEV_CGCORE_MBX_2D				= 0,
	DEV_CGCORE_MBX_ASYNC_2D			= 1,
	DEV_CGCORE_MBX_TA				= 2,
	DEV_CGCORE_MBX_3D				= 3,
	DEV_CGCORE_M24VA				= 4,
	DEV_CGCORE_PDP_GRAPHICS			= 5,
	DEV_CGCORE_PDP_OVERLAY			= 6,

	DEV_CLOCKGATE_CORE_FORCE_I32	= 0x7fffffff
} DEV_CLOCKGATE_CORE;


typedef struct _CONTIG_POOL_DESCRIPTOR_TAG
{
	char 			*name;				

	IMG_SYS_PHYADDR	BaseSysPAddr;		

	IMG_SIZE_T 		uSize;				

	void (*hook_first_alloc) (void *);	

	void (*hook_last_free) (void *);	

	void *hook_handle;					
} CONTIG_POOL_DESCRIPTOR;

typedef struct DEV_ARENA_DESCRIPTOR_TAG
{
	IMG_CHAR			*pszName;		

	IMG_DEV_VIRTADDR	BaseDevVAddr;	

	IMG_UINT32			ui32Size;		

} DEV_ARENA_DESCRIPTOR;



typedef struct _SYS_DEVICE_ID_TAG
{
	IMG_UINT32	uiID;
	IMG_BOOL	bInUse;

} SYS_DEVICE_ID;


#if defined(UBUILD)
typedef struct _SYS_DATA_TAG_
{
	IMG_UINT32					ui32InstalledISRs;			
	IMG_UINT32					ui32NumDevices;				
	IMG_BOOL					bBMInitialised;				
	SYS_DEVICE_ID				sDeviceID[SYS_DEVICE_COUNT];
	PVRSRV_DEVICE_NODE			*psDeviceNodeList;			
	PVRSRV_POWER_DEV			*psPowerDeviceList;			
	PVR_POWER_STATE 			eCurrentPowerState;			
	PVR_POWER_STATE 			eFailedPowerState;			
	IMG_UINT32					ui32CurrentOSPowerState;	
	PVRSRV_QUEUE_INFO			*psQueueList;				
	PVRSRV_SYNC_INFO 			*psSharedSyncInfoList;		
	IMG_UINT32					*pui32KickerAddr;			
	IMG_UINT32					*pui32KickerAddrUM;			
	IMG_PVOID					pvEnvSpecificData;			
	IMG_PVOID					pvSysSpecificData;			


	PVRSRV_RES_HANDLE			hQProcessResource;			
	IMG_VOID					*pvSOCTimerRegsBase;		
	IMG_UINT32					ui32SOCTimerRegsSize;
	IMG_VOID					*pvSOCClockGateRegsBase;	
	IMG_UINT32					ui32SOCClockGateRegsSize;
#if (BM_NUM_CONTIG_POOLS == 0)
	
	CONTIG_POOL_DESCRIPTOR		asContigPool[1];
#else
	CONTIG_POOL_DESCRIPTOR		asContigPool[BM_NUM_CONTIG_POOLS];
#endif
	DEV_ARENA_DESCRIPTOR		asDevMemArena[BM_NUM_DEVMEM_ARENAS];
	BM_STATE					*pBMState;					
	PFN_CMD_PROC				*ppfnCmdProcList[SYS_DEVICE_COUNT];
															


	PCOMMAND_COMPLETE_DATA		*ppsCmdCompleteData[SYS_DEVICE_COUNT];
															

	IMG_BOOL					bReProcessQueues;			


#if defined(REENTRANCY_PROTECTION)
	


	PVRSRV_RES_HANDLE			hBMResource;
	PVRSRV_RES_HANDLE			hSyncInfoResource;
	PVRSRV_RES_HANDLE			hQueueResource;
#endif
} SYS_DATA;
#endif

PVRSRV_ERROR SysInitialise(IMG_VOID);

#if defined(UBUILD)
PVRSRV_ERROR SysDeinitialise(SYS_DATA *psSysData);
#endif
PVRSRV_ERROR SysLocateDevice(IMG_VOID			*pvDevice,
							 IMG_UINT32			*pui32DevIRQ,
							 IMG_SYS_PHYADDR	*psDevSysAddr);

PVRSRV_ERROR SysInstallISR(IMG_UINT32 ui32DevIRQ, IMG_VOID *pvDevice, IMG_CPU_PHYADDR *psRegsPhysBase);

PVRSRV_ERROR SysUnInstallISR(IMG_UINT32 ui32DevIRQ, IMG_VOID *pvDevice);

PVRSRV_ERROR SysGetManagedMemoryInfo(CONTIG_POOL_DESCRIPTOR **ppsContigPool, IMG_UINT32 *pui32ContigCount,
									 DEV_ARENA_DESCRIPTOR **ppsDevPool, IMG_UINT32 *pui32DevPoolCount);

SYS_MMU_MODE SysMMUMode(IMG_VOID);

IMG_UINT32 SysGetDevicePhysOffset(IMG_VOID);

IMG_VOID SysGlobalDisableInterrupts(IMG_VOID);

IMG_VOID SysGlobalEnableInterrupts(IMG_VOID);

#ifdef PDUMP
PVRSRV_PDUMP_MEMMAP* SysGetPDUMPData();

IMG_VOID SysReleasePDUMPData(PVRSRV_PDUMP_MEMMAP *psPDData);
#endif


IMG_BOOL SysCoreQuery(IMG_UINT32 ui32Core);

PVRSRV_ERROR SysSetPowerState(PVR_POWER_STATE eNewPowerState,
							  PVR_POWER_STATE eCurrentPowerState);
PVRSRV_ERROR SysOEMFunction(IMG_UINT32	ui32ID,
							IMG_VOID	*pvIn,
							IMG_VOID	*pvOut);
#if defined(UBUILD)
extern SYS_DATA* gpsSysData;


#ifdef INLINE_IS_PRAGMA
#pragma inline(SysAcquireData)
#endif
static INLINE PVRSRV_ERROR SysAcquireData(SYS_DATA **ppsSysData)
{
	
	*ppsSysData = gpsSysData;

	



	if (gpsSysData == IMG_NULL)
	{
		return PVRSRV_ERROR_GENERIC;
	}

	return PVRSRV_OK;
}


#if defined(__cplusplus)
}
#endif
#endif
#endif 

