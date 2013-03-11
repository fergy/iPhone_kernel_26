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

#ifndef __SERVICESEXT_H__
#define __SERVICESEXT_H__

#include "pvrsrverror.h"
#define PVRSRV_LOCKFLG_READONLY     	(1)		

typedef enum _PVRSRV_DEVICE_CLASS_
{
	PVRSRV_DEVICE_CLASS_3D				= 0 ,
	PVRSRV_DEVICE_CLASS_DISPLAY			= 1 ,
	PVRSRV_DEVICE_CLASS_BUFFER			= 2 ,

	PVRSRV_DEVICE_CLASS_FORCE_I32 		= 0x7fffffff

} PVRSRV_DEVICE_CLASS;


 
typedef enum _PVRSRV_POWER_STATE_
{
	PVRSRV_POWER_Unspecified			= -1,	
	PVRSRV_POWER_STATE_D0				= 0,	
	PVRSRV_POWER_STATE_D1				= 1,	
	PVRSRV_POWER_STATE_D2				= 2,	
	PVRSRV_POWER_STATE_D3				= 3,	

	PVRSRV_POWER_STATE_FORCE_I32 = 0x7fffffff

} PVR_POWER_STATE, *PPVR_POWER_STATE;


typedef enum _PVRSRV_PIXEL_FORMAT_ {
	PVRSRV_PIXEL_FORMAT_UNKNOWN			=  0,
	PVRSRV_PIXEL_FORMAT_RGB565			=  1,
	PVRSRV_PIXEL_FORMAT_RGB555			=  2,
	PVRSRV_PIXEL_FORMAT_RGB888			=  3,
	PVRSRV_PIXEL_FORMAT_BGR888			=  4,
	PVRSRV_PIXEL_FORMAT_YUV420			=  5,
	PVRSRV_PIXEL_FORMAT_YUV444			=  6,
	PVRSRV_PIXEL_FORMAT_VUY444			=  7,
	PVRSRV_PIXEL_FORMAT_GREY_SCALE		=  8,
	PVRSRV_PIXEL_FORMAT_YUYV			=  9,
	PVRSRV_PIXEL_FORMAT_YVYU			= 10,
	PVRSRV_PIXEL_FORMAT_UYVY			= 11, 
	PVRSRV_PIXEL_FORMAT_VYUY			= 12,
	PVRSRV_PIXEL_FORMAT_PAL12			= 13,
	PVRSRV_PIXEL_FORMAT_PAL8			= 14,
	PVRSRV_PIXEL_FORMAT_PAL4			= 15,
	PVRSRV_PIXEL_FORMAT_PAL2			= 16,
	PVRSRV_PIXEL_FORMAT_PAL1			= 17,
	PVRSRV_PIXEL_FORMAT_ARGB1555		= 18,
	PVRSRV_PIXEL_FORMAT_ARGB4444		= 19, 
	PVRSRV_PIXEL_FORMAT_ARGB8888		= 20,
	PVRSRV_PIXEL_FORMAT_ABGR8888		= 21,
	PVRSRV_PIXEL_FORMAT_YV12			= 22,
	PVRSRV_PIXEL_FORMAT_I420			= 23,
 	PVRSRV_PIXEL_FORMAT_IMC2			= 24,
	
	PVRSRV_PIXEL_FORMAT_FORCE_I32 = 0x7fffffff,
} PVRSRV_PIXEL_FORMAT;


typedef enum _PVRSRV_ALPHA_FORMAT_ {
	PVRSRV_ALPHA_FORMAT_UNKNOWN		=  0x00000000,
	PVRSRV_ALPHA_FORMAT_PRE			=  0x00000001,
	PVRSRV_ALPHA_FORMAT_NONPRE		=  0x00000002,
	PVRSRV_ALPHA_FORMAT_MASK		=  0x0000000F,
} PVRSRV_ALPHA_FORMAT;

typedef enum _PVRSRV_COLOURSPACE_FORMAT_ {
	PVRSRV_COLOURSPACE_FORMAT_UNKNOWN		=  0x00000000,
	PVRSRV_COLOURSPACE_FORMAT_LINEAR		=  0x00010000,
	PVRSRV_COLOURSPACE_FORMAT_NONLINEAR		=  0x00020000,
	PVRSRV_COLOURSPACE_FORMAT_MASK			=  0x000F0000,
} PVRSRV_COLOURSPACE_FORMAT;

#define PVRSRV_CREATE_SWAPCHAIN_SHARED		(1<<0)
#define PVRSRV_CREATE_SWAPCHAIN_QUERY		(1<<1)


typedef struct PVRSRV_RESOURCE_TAG 
{
    volatile IMG_UINT32 ui32Lock;
    IMG_UINT32          ui32ID;
}PVRSRV_RESOURCE;
typedef PVRSRV_RESOURCE PVRSRV_RES_HANDLE;


typedef struct _PVRSRV_SYNC_INFO_
{
    PVRSRV_RES_HANDLE   hAccess;                
	IMG_UINT32			ui32NextWriteOp;        

     
	volatile IMG_UINT32 ui32LastWriteOp;		
 
	volatile IMG_UINT32	ui32ReadOpsComplete;    
	IMG_UINT32			ui32ReadOpsPending;     

	volatile IMG_UINT32	ui32BlitOpsPending;
	volatile IMG_UINT32	ui32BlitOpsComplete;

	struct _PVRSRV_SYNC_INFO_ *psKernSyncInfo; 
	
	struct _PVRSRV_SYNC_INFO_ *psNextKM;

	IMG_UINT32 ui32IPCKey;
	IMG_UINT32 ui32ShmID;

} PVRSRV_SYNC_INFO, *PPVRSRV_SYNC_INFO;


typedef struct _PVRSRV_SYNC_OBJECT
{
	PVRSRV_SYNC_INFO	*psSyncInfo;		
	IMG_UINT32			ui32NextWriteOp;	
	IMG_UINT32			ui32ReadOpsPending;	
}PVRSRV_SYNC_OBJECT, *PPVRSRV_SYNC_OBJECT;

typedef struct _PVRSRV_COMMAND
{
	IMG_UINT32			ui32CmdSize;		
	IMG_UINT32			ui32DevIndex;		
	IMG_UINT32			CommandType;		
	IMG_UINT32			ui32DstSyncCount;	
	IMG_UINT32			ui32SrcSyncCount;	
	PVRSRV_SYNC_OBJECT	*psDstSync;			
	PVRSRV_SYNC_OBJECT	*psSrcSync;			
	IMG_UINT32			ui32DataSize;		
	IMG_UINT32			ui32ProcessID;		
	IMG_VOID			*pvData;			
#if !defined(SUPPORT_MBX_1_5_FEATURES) && defined(SUPPORT_PRE_MBX15_COLOR_MASK)
	IMG_VOID			*pvDataUM;			
#endif
}PVRSRV_COMMAND, *PPVRSRV_COMMAND;


typedef struct _PVRSRV_QUEUE_INFO_
{
	IMG_VOID			*pvLinQueueKM;			
	IMG_VOID			*pvLinQueueUM;			
	volatile IMG_UINT32	ui32ReadOffset;			
	volatile IMG_UINT32	ui32WriteOffset;		
	PVRSRV_RES_HANDLE	hAccess;				

	IMG_UINT32			*pui32KickerAddrKM;		
	IMG_UINT32			*pui32KickerAddrUM;		
	IMG_UINT32			ui32QueueSize;			

	IMG_HANDLE			hResItem;				
	
	IMG_UINT32			ui32ProcessID;			

	IMG_UINT32                      ui32QueueStructureSize;
	IMG_UINT32                      ui32IPCKey;
	IMG_UINT32                      ui32ShmID;
	struct _PVRSRV_QUEUE_INFO_      *psQueueDaemon;
	struct _PVRSRV_QUEUE_INFO_ *psNextKM;		 
}PVRSRV_QUEUE_INFO;

typedef struct _IMG_RECT_
{
	IMG_INT32	x0;
	IMG_INT32	y0;	
	IMG_INT32	x1;	
	IMG_INT32	y1;	
}IMG_RECT;

typedef struct _IMG_RECT_16_
{
	IMG_INT16	x0;
	IMG_INT16	y0;	
	IMG_INT16	x1;	
	IMG_INT16	y1;	
}IMG_RECT_16;

typedef struct DISPLAY_DIMS_TAG
{
	IMG_UINT32	ui32ByteStride;
	IMG_UINT32	ui32Width;
	IMG_UINT32	ui32Height;
} DISPLAY_DIMS;

#define MAX_DISPLAY_DIMENSIONS	(10) 

typedef struct DISPLAY_FORMAT_TAG
{
	
	PVRSRV_PIXEL_FORMAT		pixelformat;
	
	IMG_UINT				ui32DimsCount;
	
	DISPLAY_DIMS			asDimsList[MAX_DISPLAY_DIMENSIONS];
} DISPLAY_FORMAT;

typedef struct DISPLAY_SURF_ATTRIBUTES_TAG
{
	
	PVRSRV_PIXEL_FORMAT		pixelformat;
	
	DISPLAY_DIMS			sDims;
} DISPLAY_SURF_ATTRIBUTES;


typedef struct DISPLAY_MODE_INFO_TAG
{
	
	PVRSRV_PIXEL_FORMAT		pixelformat;
	
	DISPLAY_DIMS			sDims;
	
	IMG_UINT32				ui32RefreshHZ;
	
	IMG_UINT32				ui32OEMFlags;
} DISPLAY_MODE_INFO;



#define MAX_DISPLAY_NAME_SIZE	(20) 

typedef struct DISPLAY_INFO_TAG
{
	IMG_UINT32 ui32MaxSwapChains;
	
	IMG_UINT32 ui32MaxSwapChainBuffers;

	IMG_UINT32 ui32MinSwapInterval;

	IMG_UINT32 ui32MaxSwapInterval;

	IMG_CHAR	szDisplayName[MAX_DISPLAY_NAME_SIZE];

#if defined(SUPPORT_HW_CURSOR)
	IMG_UINT16	ui32CursorWidth;
	IMG_UINT16	ui32CursorHeight;
#endif
	
} DISPLAY_INFO;


typedef struct PVRSRV_CURSOR_SHAPE_TAG
{
	IMG_UINT16			ui16Width;
	IMG_UINT16			ui16Height;
	IMG_INT16			i16XHot;
	IMG_INT16			i16YHot;
	
	
	IMG_VOID*   		pvMask;
	IMG_INT16  			i16MaskByteStride;
	
	
	IMG_VOID*			pvColour;
	IMG_INT16			i16ColourByteStride;
	PVRSRV_PIXEL_FORMAT	eColourPixelFormat; 
} PVRSRV_CURSOR_SHAPE;

#define PVRSRV_SET_CURSOR_VISIBILITY	(1<<0)
#define PVRSRV_SET_CURSOR_POSITION		(1<<1)
#define PVRSRV_SET_CURSOR_SHAPE			(1<<2)
#define PVRSRV_SET_CURSOR_ROTATION		(1<<3)

typedef struct PVRSRV_CURSOR_INFO_TAG
{
	
	IMG_UINT32 ui32Flags;
	
	
	IMG_BOOL bVisible;
	
	
	IMG_INT16 i16XPos;
	IMG_INT16 i16YPos;
	
	
	PVRSRV_CURSOR_SHAPE sCursorShape;
	
	
	IMG_UINT32 ui32Rotation;
 
} PVRSRV_CURSOR_INFO;

typedef IMG_VOID (*PFN_CMD_COMPLETE) (IMG_HANDLE);
typedef IMG_VOID (**PPFN_CMD_COMPLETE) (IMG_HANDLE);

typedef IMG_VOID (*PFN_ISR_HANDLER)(IMG_VOID*, IMG_UINT32 ui32MBXIntEnable);

typedef IMG_BOOL (*PFN_CMD_PROC) (IMG_HANDLE, IMG_UINT32, IMG_VOID*); 
typedef IMG_BOOL (**PPFN_CMD_PROC) (IMG_HANDLE, IMG_UINT32, IMG_VOID*); 

typedef IMG_BOOL (*PFN_SWAP_TO_SYSTEM) (IMG_HANDLE); 

typedef PVRSRV_ERROR (*PFN_INSERT_CMD) (PVRSRV_QUEUE_INFO*, 
										PVRSRV_COMMAND**,
										IMG_UINT32,
										IMG_UINT16,
										IMG_UINT32,
										PVRSRV_SYNC_INFO*[],
										IMG_UINT32,
										PVRSRV_SYNC_INFO*[],
										IMG_UINT32); 

typedef PVRSRV_ERROR (*PFN_SUBMIT_CMD) (PVRSRV_QUEUE_INFO*, PVRSRV_COMMAND*, IMG_BOOL);

#ifdef __linux__
typedef IMG_VOID* (*PFN_MAP_KERNEL_PTR)(IMG_HANDLE, IMG_VOID*, IMG_UINT32);
typedef IMG_BOOL (*PFN_REMOVE_MAPPING)(IMG_VOID*, IMG_UINT32);	
typedef IMG_CPU_VIRTADDR (*PFN_HOST_MAP_PHYS_TO_LIN)(IMG_CPU_PHYADDR, IMG_UINT32, IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_HOST_ALLOC_PAGES_WRAPPER)(IMG_PVOID*);
#endif


typedef struct BUFFER_INFO_TAG
{
	IMG_UINT32 			ui32BufferCount;
	IMG_UINT32			ui32BufferDeviceID;
	PVRSRV_PIXEL_FORMAT	pixelformat;
	IMG_UINT32			ui32ByteStride;
	IMG_UINT32			ui32Width;
	IMG_UINT32			ui32Height;
	IMG_UINT32			ui32OEMFlags;
} BUFFER_INFO;

typedef enum _OVERLAY_DEINTERLACE_MODE_
{
	WEAVE=0x0,
	BOB_ODD,
	BOB_EVEN,
	BOB_EVEN_NONINTERLEAVED
} OVERLAY_DEINTERLACE_MODE;


#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVGetNextWriteOp)
#endif
static INLINE
IMG_UINT32 PVRSRVGetNextWriteOp(PVRSRV_SYNC_INFO *psSyncInfo, IMG_BOOL bIsReadOp)
{
	IMG_UINT32 ui32NextOp;

	if(bIsReadOp)
	{
		ui32NextOp = psSyncInfo->ui32NextWriteOp;
	}
	else
	{      
		
		ui32NextOp = psSyncInfo->ui32NextWriteOp++;
	}

	return ui32NextOp;
}

#ifdef INLINE_IS_PRAGMA
#pragma inline(PVRSRVGetReadOpsPending)
#endif
static INLINE
IMG_UINT32 PVRSRVGetReadOpsPending(PVRSRV_SYNC_INFO *psSyncInfo, IMG_BOOL bIsReadOp)
{
	IMG_UINT32 ui32ReadOps;			

	if(bIsReadOp)
	{
        
		ui32ReadOps = psSyncInfo->ui32ReadOpsPending++;
	}
	else
	{
		ui32ReadOps = psSyncInfo->ui32ReadOpsPending;
	}

	return ui32ReadOps;
}

#endif 

