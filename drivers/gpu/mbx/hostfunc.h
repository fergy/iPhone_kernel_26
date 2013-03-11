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

#ifdef DEBUG_RELEASE_BUILD
#pragma optimize( "", off )
#define DEBUG		1
#endif

#ifndef __HOSTFUNC_H__
#define __HOSTFUNC_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include "mbxaccessapitype.h"


#define KERNEL_ID				0xffffffffL
#define POWER_MANAGER_ID		0xfffffffeL
#define ISR_ID					0xfffffffdL
#define TIMER_ID				0xfffffffcL

#define PVRSRV_HOST_HEAP_MASK			0xf 
#define PVRSRV_HOST_NON_SHAREABLE_HEAP	0x0 
#define PVRSRV_HOST_SHAREABLE_HEAP		0x1 

#define PVRSRV_HOST_MAX_HEAP			0x2 

#define CACHETYPE_UNCACHED			0x00000001 
#define CACHETYPE_CACHED			0x00000002 
#define CACHETYPE_WRITECOMBINED		0x00000004 
#define EXTRA_CACHETYPE_SHARED		0x00000008 
#define EXTRA_CACHETYPE_VIRTUAL		0x00000010 

#define PVRSRV_HAP_UNCACHED			0x00000000
#define PVRSRV_HAP_CACHED			0x00000001
#define PVRSRV_HAP_WRITECOMBINE		0x00000002
#define PVRSRV_HAP_USER_VISIBLE		0x00000004
#define PVRSRV_HAP_CACHETYPE_MASK	0x00000007


IMG_UINT32 HostClockus(IMG_VOID);
IMG_UINT32 HostGetPageSize(IMG_VOID);
PVRSRV_ERROR HostInstallISR(IMG_UINT32 ui32Irq, const IMG_CHAR *pszISRName, IMG_VOID *pvData, IMG_BOOL bIsSystemISR, IMG_CPU_PHYADDR *psRegsPhysBase);
PVRSRV_ERROR HostUnInstallISR(IMG_UINT32 ui32Irq, IMG_VOID *pvData);
IMG_CPU_PHYADDR HostMapLinToCPUPhys(IMG_VOID* pvLinAddr);
IMG_VOID HostMemCopy(IMG_VOID *pvDst, const IMG_VOID *pvSrc, IMG_UINT32 ui32Size);
IMG_VOID *HostMapPhysToLin (IMG_CPU_PHYADDR BasePAddr, IMG_UINT32 ui32Bytes, IMG_UINT32 ui32CacheType);
IMG_BOOL HostUnMapPhysToLin(IMG_VOID *pvLinAddr, IMG_UINT32 ui32Bytes);
#ifndef HostGetCurrentProcessID
IMG_UINT32 HostGetCurrentProcessID(IMG_VOID);
#endif
IMG_UINT32 HostGetCurrentThreadID(IMG_VOID);
IMG_VOID HostMemSet(IMG_VOID *pvDest, IMG_UINT8 ui8Value, IMG_UINT32 ui32Size);

IMG_BOOL HostReadRegistryString(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValue, IMG_CHAR *pszOutBuf, IMG_UINT32 ui32OutBufSize);
IMG_BOOL HostReadRegistryBinary(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValue, IMG_VOID *pszOutBuf, IMG_UINT32 *pui32OutBufSize);
IMG_BOOL HostReadRegistryInt(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValue, IMG_UINT32 *pui32Data);
IMG_BOOL HostWriteRegistryString(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValue, IMG_CHAR *pszInBuf);
IMG_BOOL HostWriteRegistryBinary(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValue, IMG_CHAR *pszInBuf, IMG_UINT32 ui32InBufSize);
PVRSRV_ERROR HostReadRegistryDWORDFromString(IMG_UINT32 ui32DevCookie, IMG_CHAR *pszKey, IMG_CHAR *pszValueName, IMG_UINT32 *pui32Data);

PVRSRV_ERROR HostAllocMem(IMG_UINT32 ui32Flags, IMG_UINT32 ui32Size, IMG_PVOID *ppvLinAddr, IMG_UINT32 ui32OwnerAddr);
PVRSRV_ERROR HostFreeMem(IMG_UINT32 ui32Flags, IMG_PVOID pvLinAddr);
PVRSRV_ERROR HostAllocPages(IMG_UINT32 ui32Flags, IMG_UINT32 ui32Size, IMG_PVOID *ppvLinAddr, IMG_UINT32 ui32OwnerAddr);
PVRSRV_ERROR HostAllocPagesWrapper(IMG_PVOID *ppvLinAddr);
PVRSRV_ERROR HostFreePages(IMG_UINT32 ui32Flags, IMG_PVOID pvLinAddr);
PVRSRV_ERROR HostInitEnvData(IMG_PVOID *ppvEnvSpecificData, IMG_UINT32 ui32MMUMode);
PVRSRV_ERROR HostDeInitEnvData(IMG_PVOID pvEnvSpecificData);
IMG_CHAR* HostStringCopy(IMG_CHAR *pszDest, const IMG_CHAR *pszSrc);
PVRSRV_ERROR HostOSPowerManagerConnect(IMG_VOID);
PVRSRV_ERROR HostOSPowerManagerDisconnect(IMG_VOID);
PVRSRV_ERROR HostSetWaitObject(PVRSRV_WAITOBJECT sWaitObject);
PVRSRV_ERROR HostDestroyWaitObject(PVRSRV_WAITOBJECT sWaitObject);
PVRSRV_ERROR HostDuplicateWaitObject(PVRSRV_WAITOBJECT sWaitObject,
								IMG_UINT32 ui32ProccessId,
								PVRSRV_WAITOBJECT * psNewWaitObject);

void HostThreadSleep(IMG_UINT32 ui32Milliseconds);

#ifdef ADJUSTWRITECOMBINING

extern PVRSRV_ERROR HostDisableWriteCombining();
extern PVRSRV_ERROR HostEnableWriteCombining();


typedef struct _WRITE_COMBINED_MEM_TAG_
{
	IMG_PVOID pvAddress;
	IMG_UINT32 ui32MapSize;
	struct _WRITE_COMBINED_MEM_TAG_ *psNext;
}WRITE_COMBINED_MEM_TAG, *PWRITE_COMBINED_MEM_TAG;

#endif

PVRSRV_ERROR HostLockResource(PVRSRV_RES_HANDLE *phResource, IMG_UINT32 ui32ID, IMG_BOOL bBlock);
PVRSRV_ERROR HostUnlockResource(PVRSRV_RES_HANDLE *phResource, IMG_UINT32 ui32ID);
IMG_BOOL HostIsResourceLocked(PVRSRV_RES_HANDLE *phResource, IMG_UINT32 ui32ID);
PVRSRV_ERROR HostCreateResource(PVRSRV_RES_HANDLE *phResource);
PVRSRV_ERROR HostDestroyResource(PVRSRV_RES_HANDLE *phResource);
IMG_VOID HostBreakResourceLock(PVRSRV_RES_HANDLE *phResource, IMG_UINT32 ui32ID);
IMG_VOID HostWaitus(IMG_UINT32 ui32Timeus);
IMG_VOID HostReleaseThreadQuanta(IMG_VOID);
IMG_UINT32 HostPCIReadDword(IMG_UINT32 ui32Bus, IMG_UINT32 ui32Dev, IMG_UINT32 ui32Func, IMG_UINT32 ui32Reg);
IMG_VOID HostPCIWriteDword(IMG_UINT32 ui32Bus, IMG_UINT32 ui32Dev, IMG_UINT32 ui32Func, IMG_UINT32 ui32Reg, IMG_UINT32 ui32Value);

#ifndef HostReadHWReg
IMG_UINT32 HostReadHWReg(IMG_PVOID pvLinRegBaseAddr, IMG_UINT32 ui32Offset);
#endif
#ifndef HostWriteHWReg
IMG_VOID HostWriteHWReg(IMG_PVOID pvLinRegBaseAddr, IMG_UINT32 ui32Offset, IMG_UINT32 ui32Value);
#endif

typedef IMG_VOID (*PFN_TIMER_FUNC)(IMG_VOID*);

IMG_HANDLE HostAddTimer(MBXTimerCBID id, IMG_VOID *pvData, IMG_UINT32 ui32MsTimeout);
PVRSRV_ERROR HostRemoveTimer (IMG_HANDLE hTimer);


#if defined(CONFIG_PCI_INIT)
typedef enum _HOST_PCI_ADDR_RANGE_FUNC_
{
	HOST_PCI_ADDR_RANGE_FUNC_LEN,
	HOST_PCI_ADDR_RANGE_FUNC_START,
	HOST_PCI_ADDR_RANGE_FUNC_END,
	HOST_PCI_ADDR_RANGE_FUNC_REQUEST,
	HOST_PCI_ADDR_RANGE_FUNC_RELEASE
}HOST_PCI_ADDR_RANGE_FUNC;

typedef enum _HOST_PCI_INIT_FLAGS_
{
	HOST_PCI_INIT_FLAG_BUS_MASTER = 0x1,
	HOST_PCI_INIT_FLAG_FORCE_I32 = 0x7fffffff
} HOST_PCI_INIT_FLAGS;

PVRSRV_ERROR HostPCIAcquireDev(IMG_VOID *pvSysData, IMG_UINT16 ui16VendorID, IMG_UINT16 ui16DeviceID, HOST_PCI_INIT_FLAGS eFlags);
IMG_UINT32   HostPCIAddrRangeLen(IMG_VOID *pvSysData, IMG_UINT32 ui32Index);
IMG_UINT32   HostPCIAddrRangeStart(IMG_VOID *pvSysData, IMG_UINT32 ui32Index);
IMG_UINT32   HostPCIAddrRangeEnd(IMG_VOID *pvSysData, IMG_UINT32 ui32Index);
PVRSRV_ERROR HostPCIRequestAddrRange(IMG_VOID *pvSysData, IMG_UINT32 ui32Index);
PVRSRV_ERROR HostPCIReleaseAddrRange(IMG_VOID *pvSysData, IMG_UINT32 ui32Index);
PVRSRV_ERROR HostPCIReleaseDev(IMG_VOID *pvSysData);
		
#endif 


#define HOST_PAGESIZE		HostGetPageSize
#define HOST_PAGEMASK		(~(HOST_PAGESIZE()-1))

#ifdef INLINE_IS_PRAGMA
#pragma inline(Host_PageAlignFn)
#endif
static INLINE
IMG_UINT32 Host_PageAlignFn(IMG_UINT32 ui32Addr)
{
	IMG_UINT32 ui32PageSize;
	ui32PageSize = HostGetPageSize();

	return ( (ui32Addr + (ui32PageSize-1)) & (~(ui32PageSize-1)) );
}

#define HOST_PAGEALIGN(addr)	Host_PageAlignFn(addr)


#if defined (__cplusplus)
}
#endif

#endif  

