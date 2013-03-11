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

#ifndef __SERVICES_H__
#define __SERVICES_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include "img_defs.h"
#include "servicesext.h"

#define PVRSRV_MAX_CMD_SIZE		1024

#define PVRSRV_MAX_DEVICES		16	

#define PVRSRV_MEMFLG_RENDERABLE		(1<<0)	
#define PVRSRV_MEMFLG_TEXTURE			(1<<1)  
#define PVRSRV_MEMFLG_ZBUFFER			(1<<2)	
#define PVRSRV_MEMFLG_CONTIGUOUS		(1<<3)	
#define PVRSRV_MEMFLG_ALTERNATE_HEAP	(1<<4)	
#define PVRSRV_MEMFLG_FLIPPABLE			(1<<5)	
#define PVRSRV_MEMFLG_SAVERESTORE		(1<<6)	
#define PVRSRV_MEMFLG_NOSYNCOBJ			(1<<7)	
#define PVRSRV_MEMFLG_PARAMBUFFER		(1<<8)	

#define PVRSRV_MEMFLG_NO_RESMAN		(IMG_UINT32)(1<<31)	

#define PVRSRV_NO_CONTEXT_LOSS					0		
#define PVRSRV_SEVERE_LOSS_OF_CONTEXT			1		
#define PVRSRV_PRE_STATE_CHANGE_MASK			0x80	


#define PVRSRV_DEFAULT_DEV_COOKIE			(1)	 


#define PVRSRVRESMAN_PROCESSID_FIND			(0xffffffff) 


#define PVRSRV_MISC_INFO_TIMER_PRESENT			(1<<0)
#define PVRSRV_MISC_INFO_CLOCKGATE_PRESENT		(1<<1)
#define PVRSRV_MISC_INFO_CMDKICKER_PRESENT		(1<<2)


typedef enum _PVRSRV_DEVICE_TYPE_
{
	PVRSRV_DEVICE_TYPE_UNKNOWN			= 0,
	PVRSRV_DEVICE_TYPE_MBX1				= 1,
	PVRSRV_DEVICE_TYPE_MBX1_LITE		= 2,

	PVRSRV_DEVICE_TYPE_M24VA			= 3,
	PVRSRV_DEVICE_TYPE_MVDA2			= 4,
	PVRSRV_DEVICE_TYPE_MVED1			= 5,

	
	PVRSRV_DEVICE_TYPE_EXT				= 6,

	PVRSRV_DEVICE_TYPE_FORCE_I32		= 0x7fffffff

} PVRSRV_DEVICE_TYPE;

#if defined(UBUILD)
typedef enum _PVR_POWER_CONTROL_
{
	PVRSRV_POWER_CONTROL_SET			= 0,	
	PVRSRV_POWER_CONTROL_RETRY			= 1,	
	PVRSRV_POWER_CONTROL_QUERY			= 2,	

	PVRSRV_POWER_CONTROL_FORCE_I32 = 0x7fffffff

} PVR_POWER_CONTROL, *PPVR_POWER_CONTROL;



typedef struct _PVRSRV_CONNECTION_
{
	IMG_HANDLE hServices;					
	IMG_UINT32 ui32ProcessID;				
}PVRSRV_CONNECTION;

#endif


#if defined(UBUILD)

typedef struct _PVRSRV_DEV_DATA_
{
	PVRSRV_CONNECTION	sConnection;		
	IMG_HANDLE			hDevCookie;			


} PVRSRV_DEV_DATA, *PPVRSRV_DEV_DATA;


typedef struct _PVRSRV_HWREG_
{
	IMG_UINT32			ui32RegAddr;	
	IMG_UINT32			ui32RegVal;		
} PVRSRV_HWREG;

typedef struct _PVRSRV_MEMBLK_  
{
	IMG_SYS_PHYADDR		sSysPhysAddr;	
	IMG_DEV_VIRTADDR	sDevVirtAddr;	
	IMG_HANDLE			hBuffer;		
	IMG_HANDLE			hResItem;		

} PVRSRV_MEMBLK; 

typedef struct _PVRSRV_MEM_INFO_
{
	struct _PVRSRV_MEM_INFO_ *psMemInfoKM;
	IMG_PVOID				pvLinAddr;			
	IMG_DEV_VIRTADDR		uiDevAddr;			
												
	IMG_UINT32				ui32Flags;			

	IMG_UINT32				ui32AllocSize;		

	PVRSRV_SYNC_INFO		*psSyncInfo;		
        IMG_UINT32 ui32IPCKeySyncInfo;
	PVRSRV_MEMBLK			sMemBlk;			
	IMG_PVOID				pvSysBackupBuffer;	
	
} PVRSRV_MEM_INFO, *PPVRSRV_MEM_INFO;


typedef struct _PVRSRV_PRIMARY_SURF_
{
	PVRSRV_MEM_INFO			*psMemInfo; 		

	IMG_BOOL				bValid;				
	IMG_UINT32				ui32ByteStride;		

	IMG_UINT32 				ui32PixelWidth;		
	IMG_UINT32 				ui32PixelHeight;	
	PVRSRV_PIXEL_FORMAT		ePixelFormat;		
} PVRSRV_PRIMARY_SURF;


typedef struct _PVRSRV_SURF_
{
	IMG_HANDLE					hBuffer;			
	struct _PVRSRV_MEM_INFO_	*psMemInfo; 		

	IMG_UINT32					ui32ByteStride;		

	IMG_UINT32 					ui32PixelWidth;		
	IMG_UINT32 					ui32PixelHeight;	
	PVRSRV_PIXEL_FORMAT			ePixelFormat;		
} PVRSRV_SURF;
#endif

typedef struct _PVRSRV_DEVICE_IDENTIFIER_
{
	PVRSRV_DEVICE_TYPE		eDeviceType;		
	PVRSRV_DEVICE_CLASS		eDeviceClass;		
	IMG_UINT32				ui32DeviceIndex;	

} PVRSRV_DEVICE_IDENTIFIER;

#if defined(UBUILD)
typedef struct _PVRSRV_MISC_INFO_
{
	IMG_UINT32	ui32StatePresent;		
	IMG_VOID	*pvSOCTimerRegs;		
	IMG_UINT32	ui32SOCTimerRegsSize;
	IMG_VOID	*pvSOCClockGateRegs;	
	IMG_UINT32	ui32SOCClockGateRegsSize;
	IMG_UINT32	*pui32CmdKicker;		
	
	
} PVRSRV_MISC_INFO;


IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVConnect(PVRSRV_CONNECTION *psConnection);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVDisconnect(PVRSRV_CONNECTION *psConnection);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVEnumerateDevices(PVRSRV_CONNECTION 			*psConnection,
												 IMG_UINT32 				*puiNumDevices,
												 PVRSRV_DEVICE_IDENTIFIER	*puiDevIDs);
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVAcquireDeviceData(PVRSRV_CONNECTION		*psConnection,
												  IMG_UINT32			uiDevIndex,
												  PVRSRV_DEV_DATA		*psDevData,
												  PVRSRV_DEVICE_TYPE	eDeviceType);

#endif


#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVGetMiscInfo(PVRSRV_CONNECTION	*psConnection,
											PVRSRV_MISC_INFO	*psMiscInfo);
#endif


#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVReleaseMiscInfo(PVRSRV_MISC_INFO *psMiscInfo);
#endif

#if defined(UBUILD)
IMG_IMPORT
IMG_VOID WriteHWRegs(IMG_PVOID pvLinRegBaseAddr, IMG_UINT32 ui32Count, PVRSRV_HWREG *psHWRegs);
#endif

IMG_IMPORT 
PVRSRV_ERROR PollForValue(volatile IMG_UINT32	*pui32LinMemAddr,
						  IMG_UINT32 			ui32Value,
						  IMG_UINT32 			ui32Mask,
						  IMG_UINT32 			ui32Waitus,
						  IMG_UINT32 			ui32Tries);

#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVAllocDeviceMem(PVRSRV_DEV_DATA	*psDevData,
											   IMG_UINT32		 ui32Flags,
											   IMG_UINT32		 ui32Size,
											   IMG_UINT32		 ui32Alignment,
											   PVRSRV_MEM_INFO	**ppsMemInfo);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVFreeDeviceMem(PVRSRV_DEV_DATA *psDevData,
											  PVRSRV_MEM_INFO *psMemInfo);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVGetFreeDeviceMem(PVRSRV_DEV_DATA	*psDevData,
												 IMG_UINT32			ui32Flags,
												 IMG_UINT32			*pui32Total,
												 IMG_UINT32			*pui32Free,
												 IMG_UINT32			*pui32LargestBlock);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVDeviceRemapMemInfo(PVRSRV_DEV_DATA	*psOwningDevData,
												   PVRSRV_MEM_INFO	*psMemInfo,
												   IMG_UINT32 		uFlags,
												   PVRSRV_DEV_DATA	*psTargetDevData,
												   IMG_UINT32		*pui32RemappedAddr);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVCreateCommandQueue(PVRSRV_DEV_DATA		*psDevData,
												   IMG_UINT32 			ui32QueueSize,
												   PVRSRV_QUEUE_INFO	**ppsQueueInfo);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVDestroyCommandQueue(PVRSRV_DEV_DATA		*psDevData,
													PVRSRV_QUEUE_INFO	*psQueueInfo);
#endif

IMG_IMPORT
PVRSRV_ERROR PVRSRVInsertCommand(PVRSRV_QUEUE_INFO	*psQueue,
								 PVRSRV_COMMAND		**ppsCommand,
								 IMG_UINT32			ui32DevIndex,
								 IMG_UINT16			CommandType,
								 IMG_UINT32			ui32DstSyncCount,
								 PVRSRV_SYNC_INFO	*apsDstSync[],
								 IMG_UINT32			ui32SrcSyncCount,
								 PVRSRV_SYNC_INFO	*apsSrcSync[],
								 IMG_UINT32			ui32DataByteSize );
		
IMG_IMPORT
PVRSRV_ERROR PVRSRVSubmitCommand(PVRSRV_QUEUE_INFO	*psQueue,
								 PVRSRV_COMMAND		*psCommand,
								 IMG_BOOL 			bReleaseQueue);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVFlushQueue(PVRSRV_QUEUE_INFO *psQueueInfo);

IMG_IMPORT 
PVRSRV_ERROR IMG_CALLCONV PVRSRVResManConnect(IMG_UINT32 ui32ProcID, IMG_BOOL bConnect);
#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPowerControl(PVRSRV_CONNECTION	*psConnection,
											 PVR_POWER_CONTROL	eControlMode,
											 PVR_POWER_STATE	*pePVRPowerState);



IMG_IMPORT
PVRSRV_ERROR PVRSRVEnumerateDeviceClass(PVRSRV_CONNECTION	*psConnection, 
										PVRSRV_DEVICE_CLASS	DeviceClass, 
										IMG_UINT32			*pui32DevCount,
										IMG_UINT32			*pui32DevID );
											
IMG_IMPORT
IMG_HANDLE OpenDisplayClassDevice(PVRSRV_CONNECTION	*psConnection, IMG_UINT32 ui32DeviceID);

IMG_IMPORT
PVRSRV_ERROR CloseDisplayClassDevice(PVRSRV_CONNECTION	*psConnection, IMG_HANDLE hDevice);

IMG_IMPORT
PVRSRV_ERROR DisplayClassDisableVsyncISR(IMG_VOID);

IMG_IMPORT
PVRSRV_ERROR DisplayClassEnableVSyncISR(IMG_VOID);

#endif


IMG_IMPORT
PVRSRV_ERROR DisplayClassEnumerateFormats(IMG_HANDLE		hDevice,
										  IMG_UINT32 		*pui32Count,
										  DISPLAY_FORMAT	*psFormat);
											
IMG_IMPORT
PVRSRV_ERROR DisplayClassEnumerateDims(IMG_HANDLE		hDevice,
									   IMG_UINT32 		*pui32Count,
									   DISPLAY_FORMAT	*psFormat,
									   DISPLAY_DIMS		**ppsDims);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSetMode(IMG_HANDLE			hDevice,
								 SYSTEM_ADDR 		*psSysBusAddr,
								 DISPLAY_MODE_INFO	*psModeInfo);

#ifdef SUPPORT_OEM_FUNCTION
IMG_IMPORT
PVRSRV_ERROR IMG_EXTERNAL DisplayClassOEMFunc(IMG_HANDLE	hDevice,
											  IMG_UINT32	ui32CmdID,
											  IMG_BYTE*		pbInData,
											  IMG_UINT32	ui32pbInDataSize,
											  IMG_BYTE*		pbOutData,
											  IMG_UINT32	ui32pbOutDataSize);
#endif

IMG_IMPORT
PVRSRV_ERROR DisplayClassCreateSwapChain(IMG_HANDLE					hDevice,
										 IMG_UINT32					ui32Flags,
										 DISPLAY_SURF_ATTRIBUTES	*psDstSurfAttrib,
										 DISPLAY_SURF_ATTRIBUTES	*psSrcSurfAttrib,
										 IMG_UINT32					ui32BufferCount,
										 IMG_UINT32					ui32OEMFlags,
										 IMG_UINT32					*pui32SwapChainID,
										 IMG_HANDLE					*phSwapChain);

IMG_IMPORT
PVRSRV_ERROR DisplayClassDestroySwapChain(IMG_HANDLE	hDevice,
										  IMG_HANDLE	hSwapChain);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSetDstRect(IMG_HANDLE	hDevice,
									IMG_HANDLE	hSwapChain,
									IMG_RECT	*psDstRect);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSetSrcRect(IMG_HANDLE	hDevice,
									IMG_HANDLE	hSwapChain,
									IMG_RECT	*psSrcRect);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSetDstColourKey(IMG_HANDLE	hDevice,
										 IMG_HANDLE	hSwapChain,
										 IMG_UINT32	ui32CKColour);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSetSrcColourKey(IMG_HANDLE	hDevice,
										 IMG_HANDLE	hSwapChain,
										 IMG_UINT32	ui32CKColour);
IMG_IMPORT
PVRSRV_ERROR DisplayClassGetSystemBuffer(IMG_HANDLE	hDevice, 
										 IMG_HANDLE	*phBuffer);
										
IMG_IMPORT
PVRSRV_ERROR DisplayClassGetBuffers(IMG_HANDLE	hDevice, 
									IMG_HANDLE	hSwapChain, 
									IMG_HANDLE	ahBuffer[]);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSwapToBuffer(IMG_HANDLE	hDevice,
									  IMG_HANDLE	hBuffer,
									  IMG_UINT32	ui32ClipRectCount,
									  IMG_RECT		*psClipRect,
									  IMG_UINT32	ui32SwapInterval,
									  IMG_HANDLE	hPrivateTag);

IMG_IMPORT
PVRSRV_ERROR DisplayClassSwapToSystem(IMG_HANDLE	hDevice,
									  IMG_HANDLE	hSwapChain);
										
IMG_IMPORT
PVRSRV_ERROR DisplayClassGetSysBusAddr(IMG_HANDLE	hDevice,
									   IMG_HANDLE	hBuffer,
									   SYSTEM_ADDR	**ppsSysBusAddr);

IMG_IMPORT
PVRSRV_ERROR DisplayClassGetCPULinAddr(IMG_HANDLE		hDevice,
									   IMG_HANDLE		hBuffer,
									   IMG_CPU_VIRTADDR	*psCPULinAddr);

IMG_IMPORT
PVRSRV_ERROR DisplayClassGetSyncObject(IMG_HANDLE		hDevice,
									   IMG_HANDLE		hBuffer,
									   PVRSRV_SYNC_INFO	**ppsSyncObj);

IMG_IMPORT
PVRSRV_ERROR DisplayClassGetDisplayInfo(IMG_HANDLE 		hDevice,
										DISPLAY_INFO* 	psDisplayInfo);

IMG_IMPORT
PVRSRV_ERROR DisplayClassLockBuffer(IMG_HANDLE 			hDevice,
									IMG_HANDLE 			hBuffer,
									IMG_RECT* 			psRect,
									IMG_UINT32 			ui32RWFlags,
									IMG_CPU_VIRTADDR*	psCPUVAddr,
									IMG_INT32* 			pi32Stride,
									IMG_UINT32* 		pui32Attributes);

IMG_IMPORT
PVRSRV_ERROR DisplayClassUnLockBuffer(IMG_HANDLE	hDevice,
									  IMG_HANDLE	hBuffer);
#if defined (SUPPORT_HW_CURSOR)
IMG_IMPORT
PVRSRV_ERROR DisplayClassSetCursorState(IMG_HANDLE hDevice, PVRSRV_CURSOR_INFO *psCursorState);
#endif

#if defined(UBUILD)
IMG_IMPORT
IMG_HANDLE OpenBufferClassDevice(PVRSRV_CONNECTION *psConnection, IMG_UINT32 ui32DeviceID);

IMG_IMPORT
PVRSRV_ERROR CloseBufferClassDevice(PVRSRV_CONNECTION *psConnection, IMG_HANDLE hDevice);
#endif
IMG_IMPORT
PVRSRV_ERROR BufferClassGetBufferInfo(IMG_HANDLE	hDevice,
									  BUFFER_INFO	*psBuffer);

IMG_IMPORT
PVRSRV_ERROR BufferClassGetSysBusAddr(IMG_HANDLE 	hDevice,
									  IMG_UINT32 	ui32BufferIndex,
									  SYSTEM_ADDR	**ppsSysBusAddr);

IMG_IMPORT
PVRSRV_ERROR BufferClassGetSyncObject(IMG_HANDLE 		hDevice,
									  IMG_UINT32 		ui32BufferIndex,
									  PVRSRV_SYNC_INFO	**ppsSyncObj);

IMG_IMPORT
PVRSRV_ERROR BufferClassLockBuffer(IMG_HANDLE 			hDevice,
								   IMG_UINT32 			ui32BufferIndex,
								   IMG_RECT* 			psRect,
								   IMG_UINT32 			ui32RWFlags,
								   IMG_CPU_VIRTADDR*	psCPUVAddr,
								   IMG_INT32* 			pi32Stride,
								   IMG_UINT32*	 		pui32OEMAttributes);

IMG_IMPORT
PVRSRV_ERROR BufferClassUnLockBuffer(IMG_HANDLE	hDevice,
									 IMG_UINT32	ui32BufferIndex);



#ifdef PDUMP2

 
#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpInit(PVRSRV_CONNECTION *psConnection);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpMemPol(PVRSRV_CONNECTION *psConnection,
										   PVRSRV_MEM_INFO *psMemInfo,
										   IMG_UINT32 ui32Offset,
										   IMG_UINT32 ui32Value,
										   IMG_UINT32 ui32Mask,
										   IMG_BOOL bLastFrame,
										   IMG_BOOL bOverwrite);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpSyncPol(PVRSRV_CONNECTION *psConnection,
										     PVRSRV_SYNC_INFO  *psClientSyncInfo,
										     IMG_BOOL           bIsRead,
										     IMG_UINT32         ui32Value,
										     IMG_UINT32			ui32Mask);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpMem(   PVRSRV_DEV_DATA    *psDevData,
											PVRSRV_CONNECTION  *psConnection,
											IMG_PVOID			pvAltLinAddr,
											PVRSRV_MEM_INFO	   *psMemInfo,
								 			IMG_UINT32			ui32Offset,
								 			IMG_UINT32			ui32Bytes,
											IMG_CHAR		   *pszComment,
								 			IMG_UINT32			ui32Flags,
											IMG_BOOL			bContinuous);

IMG_IMPORT
PVRSRV_ERROR PVRSRVPDumpWriteSlavePort( PVRSRV_CONNECTION *psConnection,
										PVRSRV_MEM_INFO   *psMemInfo,
										IMG_UINT32        ui32Bytes,
										IMG_UINT32        ui32Flags,
										IMG_BOOL          bTerminate,
										IMG_UINT32        ui32TerminateValue,
										IMG_BOOL          bContinuous);
									   
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpSync(PVRSRV_CONNECTION *psConnection,
										IMG_PVOID pvAltLinAddr,
										PVRSRV_SYNC_INFO *psClientSyncInfo,
								 		IMG_UINT32 ui32Offset,
								 		IMG_UINT32 ui32Bytes);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpReg(PVRSRV_CONNECTION *psConnection,
								 			IMG_UINT32 ui32RegAddr,
											IMG_UINT32 ui32RegValue,
								 			IMG_UINT32 ui32Flags,
											IMG_BOOL   bContinuous);
IMG_IMPORT
IMG_BOOL PVRSRVPDumpRegArray(PVRSRV_CONNECTION  *psConnection,
							 IMG_UINT32          ui32Count, 
							 PVRSRV_HWREG       *psHWRegs,
							 IMG_UINT32          ui32Flags,
							 IMG_BOOL            bContinuous);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpRegPol(PVRSRV_CONNECTION *psConnection,
								 			IMG_UINT32 ui32RegAddr,
											IMG_UINT32 ui32RegValue,
								 			IMG_UINT32 ui32Mask,
											IMG_BOOL   bContinuous);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpRegPoll(PVRSRV_CONNECTION  *psConnection,
											 IMG_UINT32			ui32RegAddr, 
											 IMG_UINT32			ui32RegValue, 
											 IMG_UINT32			ui32Mask, 
											 IMG_UINT32			ui32Flags, 
											 IMG_UINT32			ui32A, 
											 IMG_UINT32			ui32B, 
											 IMG_UINT32			ui32C);
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpMemPages(PVRSRV_CONNECTION *psConnection,
								 				IMG_HANDLE			hKernelMemInfo,
								 				IMG_DEV_PHYADDR		*pPages,
												IMG_UINT32			ui32NumPages,
												IMG_DEV_VIRTADDR	sDevAddr,
												IMG_UINT32			ui32Start,
												IMG_UINT32			ui32Length,
												IMG_BOOL			bContinuous);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpSetFrame(PVRSRV_CONNECTION *psConnection,
								 			  IMG_UINT32 ui32Frame);
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpIsLastFrame(PVRSRV_CONNECTION *psConnection,
												 IMG_BOOL *pbIsLastFrame);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpComment(PVRSRV_CONNECTION *psConnection,
								 			 IMG_CHAR *pszComment,
											 IMG_BOOL bContinuous);


IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpIsCapturing(PVRSRV_CONNECTION *psConnection,
								 				IMG_BOOL *pbIsCapturing);


IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpRegRead(PVRSRV_CONNECTION *psConnection,
								 			IMG_CHAR *pszFileName,
											IMG_UINT32 ui32FileOffset,
											IMG_UINT32 ui32Address,
											IMG_UINT32 ui32Size,
											IMG_UINT32 ui32PDumpFlags);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVPDumpParamBuffer(PVRSRV_DEV_DATA		*psDevData,
											PVRSRV_CONNECTION	*psConnection);
#endif
#endif 

typedef IMG_UINT32 PVRSRV_WAITOBJECT;

#if defined(UBUILD)
IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVCreateWaitObject(PVRSRV_DEV_DATA	*psDevData,
												 PVRSRV_WAITOBJECT	*psWaitObject);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVWaitForWaitObject(PVRSRV_DEV_DATA	*psDevData,
												  PVRSRV_WAITOBJECT	sWaitObject, 
												  IMG_UINT 			ui32MsTimeout);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVSetWaitObject(PVRSRV_DEV_DATA	*psDevData,
											  PVRSRV_WAITOBJECT	sWaitObject);

IMG_IMPORT
PVRSRV_ERROR IMG_CALLCONV PVRSRVDestroyWaitObject(PVRSRV_DEV_DATA	*psDevData,
												  PVRSRV_WAITOBJECT	sWaitObject);

#endif
typedef volatile IMG_UINT32				PVRSRV_MUTEX_HANDLE;

#define TIME_NOT_PASSED_UINT32(a,b,c)		((a - b) < c)

#ifdef INLINE_IS_PRAGMA
#pragma inline(ReadHWReg)
#endif
static FORCE_INLINE
IMG_UINT32 ReadHWReg(IMG_PVOID pvLinRegBaseAddr, IMG_UINT32 ui32Offset)
{
	return *(volatile IMG_UINT32*)((IMG_UINT32)pvLinRegBaseAddr + ui32Offset);
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(WriteHWReg)
#endif
static FORCE_INLINE
IMG_VOID WriteHWReg(IMG_PVOID pvLinRegBaseAddr, IMG_UINT32 ui32Offset, IMG_UINT32 ui32Value)
{
	*(IMG_UINT32*)((IMG_UINT32)pvLinRegBaseAddr + ui32Offset) = ui32Value;
}


#if defined (__cplusplus)
}
#endif
#endif 

