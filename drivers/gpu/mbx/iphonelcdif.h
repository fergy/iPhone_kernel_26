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

#if !defined (__IPHONELCDIF_H_)
#define __IPHONELCDIF_H_

#ifdef SUPPORT_OEM_FUNCTION
#include "iphonelcdoemapi.h"
#endif

#ifdef __linux__
//FIXME!!//////////////////////////////////////
#define IPHONELCDIF_PATH	"/dev/multitouch0"
#endif
///////////////////////////////////////////////

#define IPHONELCD_FLIP_COMMAND 			(0)
#define IPHONELCD_COMMAND_COUNT			(1)

#define IPHONELCD_MAXFORMATS				(1)
#define IPHONELCD_MAXDIMS					(1)

typedef struct IPHONELCD_PRIVATE_FLIP_CMD_TAG
{
	IMG_UINT32			ui32CmdSize;
	IMG_HANDLE			hDevHandle;
	IMG_UINT32			ui32FlipInterval;
	IMG_UINT32			ui32DevAddr;
#ifdef BUFFER_OVERLAY
	IMG_BOOL			bOverlay;
#endif
} IPHONELCD_PRIVATE_FLIP_CMD;

typedef struct IPHONELCD_BUFFER_TAG
{
	IMG_HANDLE					hSwapChain;
	PVRSRV_SYNC_INFO			*psSyncObj;
	IMG_HANDLE					hMemChunk;
	SYSTEM_ADDR					sSysAddr;
	IMG_CPU_VIRTADDR			sCPUVAddr;
	IMG_UINT32					ui32LastLockOp;
	IMG_UINT32					ui32ReadOpsPending;
	IMG_BOOL					bLocked;
	IMG_BOOL					bLockPending;
	struct IPHONELCD_BUFFER_TAG	*psNext;
} IPHONELCD_BUFFER;


typedef struct IPHONELCD_CLIENT_DEVINFO_TAG
{
	


	IMG_HANDLE					hPVRSRVHandle;
	
	
	IMG_HANDLE					hKernelDriverHandle;	

	
	IMG_UINT32					ui32NumFormats;

	
	DISPLAY_FORMAT				asDisplayFormatList[IPHONELCD_MAXFORMATS];

	
	IMG_UINT32					ui32DeviceID;

	
	IPHONELCD_BUFFER				sSystemBuffer;
	DISPLAY_FORMAT 				sSysFormat; 

	
	PFN_INSERT_CMD				pfnInsertCommand;
	PFN_SUBMIT_CMD				pfnSubmitCommand;

	
	IMG_HANDLE					hDevInfo;

	
	DISPLAY_INFO				sDisplayInfo;

	
	IMG_RECT 					sDstRect;
	IMG_RECT 					sSrcRect;
	
	
	IMG_UINT32 					ui32DstCKColour;
	IMG_UINT32 					ui32SrcCKColour;
	
	PVRSRV_DC_UMJTABLE            sMBXJumpTable;
	PFN_CMD_COMPLETE     pfnCmdComplete;

}  IPHONELCD_CLIENT_DEVINFO;


#define IPHONELCD_MAX_BACKBUFFERS		(2)
#define IPHONELCD_MAX_DIMS			(1)
#define IPHONELCD_MAX_FORMATS			(1)


struct IPHONELCD_SWAP_CHAIN_TAG;

typedef struct IPHONELCD_SWAP_CHAIN_TAG
{
	
	IMG_UINT32		ui32BufferCount;

	
	IPHONELCD_BUFFER		sSystemBuffer;

	
	IPHONELCD_BUFFER		asBufferList[IPHONELCD_MAX_BACKBUFFERS + 1];

	
	IMG_HANDLE		hSwapChain;

	
	IMG_HANDLE		hDevice;
	
	
	IMG_UINT32		ui32SwapChainID;
	
	
	PVRSRV_QUEUE_INFO *psQueueKM;
#ifdef BUFFER_OVERLAY
	
	IMG_BOOL bOverlay;
#endif
	
} IPHONELCD_SWAP_CHAIN;

typedef struct IPHONELCD_VSYNC_FLIP_ITEM_TAG
{
	IMG_HANDLE	hCmdComplete;
	IMG_UINT32	ui32DevAddr;
	IMG_UINT32	ui32SwapInterval;
	IMG_BOOL	bValid;
	IMG_BOOL	bFlipped;
	IMG_BOOL	bCmdCompleted;

}IPHONELCD_VSYNC_FLIP_ITEM;

typedef struct IPHONELCD_FBINFO_TAG
{
	SYSTEM_ADDR					sSysAddr;
	IMG_CPU_VIRTADDR			sCPUVAddr;
	IMG_UINT32					ui32Width;
	IMG_UINT32					ui32Height;
	IMG_UINT32					ui32ByteStride;
	PVRSRV_PIXEL_FORMAT			ePixelFormat;
	IMG_VOID					*pvRegs;   
	IMG_VOID					*pvRegsUM;   
	IMG_VOID					*pvRegsPhys;   
	IMG_UINT32                          ui32RegsSize;
}IPHONELCD_FBINFO;

typedef struct IPHONELCD_DEVINFO_TAG
{
	
	IPHONELCD_CLIENT_DEVINFO 		sClientDevInfo;
	
	
	
	
	PVRSRV_DC_KMJTABLE			sPVRDCJTable;

	


	IMG_HANDLE					hPVRServices;

	
	PFN_CMD_COMPLETE 			pfnCmdComplete;

	
	PFN_CMD_COMPLETE 			pfnCmdCompleteKM;
	
	
	IMG_HANDLE					hCmdCookie;
		
	
	IMG_HANDLE					hVSyncEvent;

	
	IMG_HANDLE					hVSyncThread;

	
	IPHONELCD_BUFFER				asBackBuffers[IPHONELCD_MAX_BACKBUFFERS];
	DISPLAY_FORMAT 				sBackBufferFormat[IPHONELCD_MAX_FORMATS]; 

#ifdef BUFFER_OVERLAY
	
	IPHONELCD_BUFFER				asOvlBackBuffers[IPHONELCD_MAX_BACKBUFFERS];
#endif
	
	IPHONELCD_FBINFO				sFBInfo;

	
	IPHONELCD_VSYNC_FLIP_ITEM		asVsyncFlips[IPHONELCD_MAX_BACKBUFFERS];

	
	IMG_UINT32					ui32IphoneLCDSysIntr; //FIXME

	IMG_UINT32 					ui32InsertIndex;
	
	IMG_UINT32 					ui32RemoveIndex;
	
	IMG_UINT32					ui32RefCount;
	
	IPHONELCD_SWAP_CHAIN			*psSwapChain;

#ifdef BUFFER_OVERLAY
	IPHONELCD_SWAP_CHAIN			*psOvlSwapChain;
#endif
	
	IMG_BOOL				    bIsPoweredDown;
	IMG_UINT32                  ui32LastIgoredSurfaceAddr;
	IMG_UINT32                  ui32LastQueuedSurfaceAddr;

	IMG_UINT32                  ui32FlipCompleteID;
	IMG_UINT32                  ui32FlipCompleteInProgress;

	IMG_UINT32                  ui32FlipTokenUser;
	IMG_UINT32                  ui32FlipTokenKernel;

}  IPHONELCD_DEVINFO;

typedef struct IPHONELCD_BRIDGE_PACKAGE_TAG {

	IMG_UINT32				ui32BridgeID;			
	IMG_UINT32				ui32Size;				
	IMG_VOID 				*pvParamIn;				 
	IMG_UINT32				ui32InBufferSize;		
	IMG_VOID				*pvParamOut;			
	IMG_UINT32				ui32OutBufferSize;		

}IPHONELCD_BRIDGE_PACKAGE;


typedef struct IPHONELCD_GET_CLIENTINFO_OUT_DATA_TAG
{
	PVRSRV_ERROR			eError;
	IPHONELCD_CLIENT_DEVINFO 	sClientDevInfo;
} IPHONELCD_GET_CLIENTINFO_OUT_DATA;

typedef struct IPHONELCD_GET_DEVINFO_OUT_DATA_TAG
{
	IPHONELCD_DEVINFO   sDevInfo;
} IPHONELCD_GET_DEVINFO_OUT_DATA;

typedef struct IPHONELCD_SET_MODE_INOUT_DATA_TAG
{
	
	IMG_HANDLE 				hDevInfo;
	SYSTEM_ADDR				sSysBusAddr;
	DISPLAY_MODE_INFO		sModeInfo;
	
	PVRSRV_ERROR			eError;
} IPHONELCD_SET_MODE_INOUT_DATA;



typedef struct IPHONELCD_CREATE_SWAP_CHAIN_INOUT_DATA_TAG
{
	
	IMG_HANDLE 				hDevInfo;
	IMG_UINT32				ui32BufferCount;
	DISPLAY_SURF_ATTRIBUTES	sDstSurfAttrib;
	DISPLAY_SURF_ATTRIBUTES	sSrcSurfAttrib;
	IMG_UINT32				ui32Flags;
#ifdef BUFFER_OVERLAY
	IMG_UINT32				ui32OEMFlags;
#endif

	
	PVRSRV_ERROR			eError;
	IPHONELCD_SWAP_CHAIN		sIPHONELCDSwapChain;	
	
	IMG_UINT32				ui32SwapChainID;
} IPHONELCD_CREATE_SWAP_CHAIN_INOUT_DATA;



typedef struct IPHONELCD_DESTROY_SWAP_CHAIN_INOUT_DATA_TAG
{
	
	IMG_HANDLE				hSwapChain;
	
	PVRSRV_ERROR			eError;
} IPHONELCD_DESTROY_SWAP_CHAIN_INOUT_DATA;

typedef struct IPHONELCD_IS_POWER_DOWN_OUT_DATA_TAG
{
	IMG_BOOL bResult;
} IPHONELCD_IS_POWER_DOWN_OUT_DATA;

typedef struct IPHONELCD_INIT_MAIN_IN_DATA_TAG
{
	IMG_UINT32 pvKernelPageDevInfo;
} IPHONELCD_INIT_MAIN_IN_DATA;

#ifdef SUPPORT_OEM_FUNCTION
typedef struct IPHONELCD_OEM_FUNC_INOUT_DATA_TAG
{
	
	IMG_HANDLE 				hDevInfo;
	IMG_UINT32              ui32CmdID;
	IMG_UINT32              ui32pbDataSize;

	
	PVRSRV_ERROR			eError;
	
	IMG_BYTE*               pbData;
	
} IPHONELCD_OEM_FUNC_INOUT_DATA;
#endif 



#define IPHONELCDIO_GET_CLIENTINFO		_IOWR('F', 0x31, struct IPHONELCD_GET_CLIENTINFO_OUT_DATA_TAG)
#define IPHONELCDIO_CREATE_SWAPCHAIN		_IOWR('F', 0x32, struct IPHONELCD_CREATE_SWAP_CHAIN_INOUT_DATA_TAG)
#define IPHONELCDIO_DESTROY_SWAPCHAIN		_IOWR('F', 0x33, struct IPHONELCD_DESTROY_SWAP_CHAIN_INOUT_DATA_TAG)
#define IPHONELCDIO_SET_MODE				_IOWR('F', 0x34, struct IPHONELCD_SET_MODE_INOUT_DATA_TAG)

#define IPHONELCDIO_IS_POWER_DOWN                _IOWR('F', 0x35, struct IPHONELCD_IS_POWER_DOWN_OUT_DATA_TAG)
#define IPHONELCDIO_GET_DEVINFO        _IOWR('F', 0x36, struct IPHONELCD_GET_DEVINFO_OUT_DATA_TAG)
#define IPHONELCDIO_INIT_MAIN        _IOWR('F', 0x37, struct IPHONELCD_INIT_MAIN_IN_DATA_TAG)
#define IPHONEPLCDIO_DEINIT_MAIN        _IOWR('F', 0x38, struct IPHONELCD_INIT_MAIN_IN_DATA_TAG)
#define IPHONELCDIO_UNINSTALL_VSYNC_ISR        _IOWR('F', 0x39, struct IPHONELCD_INIT_MAIN_IN_DATA_TAG)
#define IPHONELCDIO_INSTALL_VSYNC_ISR        _IOWR('F', 0x40, struct IPHONELCD_INIT_MAIN_IN_DATA_TAG)

#endif 

