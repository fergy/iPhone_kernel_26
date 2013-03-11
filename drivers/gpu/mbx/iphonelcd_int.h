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

#if !defined (__IPHONELCDINT_H_)
#define __IPHONELCDINT_H_

#if defined (__cplusplus)
extern "C" {
#endif

#define IPHONELCD_XRES			320
#define IPHONELCD_YRES			480

#define IPHONELCD_PIXEL_FORMAT	PVRSRV_PIXEL_FORMAT_RGB565
#define IPHONELCD_BITSPERPIXEL	16


#define IPHONELCD_IRQ			25

#if defined (CONFIG_IPHONE_3G) || defined(CONFIG_IPHONE_2G)
#define IPHONELCD_SYSCONFIG			0x0410
#define IPHONELCD_CONFIG				0x0444
#define IPHONELCD_DEFAULT_COLOR0		0x044C
#define IPHONELCD_TIMING_H			0x0464
#define IPHONELCD_TIMING_V			0x0468
#define IPHONELCD_POL_FREQ			0x046C
#define IPHONELCD_DIVISOR				0x0470
#define IPHONELCD_SIZE_DIG			0x0478
#define IPHONELCD_SIZE_LCD			0x047C
#define IPHONELCD_GFX_POSITION		0x0488
#define IPHONELCD_GFX_SIZE			0x048C
#define IPHONELCD_GFX_ATTRIBUTES		0x04a0
#define IPHONELCD_GFX_FIFO_THRESHOLD	0x04a4
#define IPHONELCD_GFX_WINDOW_SKIP		0x04b4
#endif

#define IPHONELCD_IRQSTATUS		0x0418
#define IPHONELCD_IRQENABLE		0x041c
#define IPHONELCD_CONTROL			0x0440
#define IPHONELCD_GFX_BA0			0x0480
#define IPHONELCD_GFX_BA1			0x0484
#define IPHONELCD_GFX_ROW_INC		0x04ac
#define IPHONELCD_GFX_PIX_INC		0x04b0
#define IPHONELCD_VID1_BA0		0x04bc
#define IPHONELCD_VID1_BA1		0x04c0
#define IPHONELCD_VID1_ROW_INC	0x04d8
#define IPHONELCD_VID1_PIX_INC	0x04dc

#define IPHONE_CONTROL_GODIGITAL		(1 << 6)
#define IPHONE_CONTROL_GOLCD			(1 << 5)
#define IPHONE_CONTROL_DIGITALENABLE	(1 << 1)
#define IPHONE_CONTROL_LCDENABLE		(1 << 0)

#define IPHONELCD_INTMASK_VSYNC	((1 << 1) | (1 << 2) | (1 << 3))
#define IPHONELCD_INTMASK_OFF	0

#if defined(USE_NON_FBDEV_ALLOC)
#define FB_SIZE (IPHONELCD_XRES * IPHONELCD_YRES * (IPHONELCD_BITSPERPIXEL / 8))
#endif

extern IMG_UINT32 gui32BufferCount;

IPHONELCD_DEVINFO * GetAnchorPtr(IMG_VOID);
PVRSRV_ERROR InitMain(IMG_VOID *pvKernelPage);
PVRSRV_ERROR DeinitMain(IMG_VOID);

PVRSRV_ERROR Init(IMG_VOID);
PVRSRV_ERROR Deinit(IMG_VOID);
IMG_IMPORT PVRSRV_ERROR GetClientInfo(IPHONELCD_CLIENT_DEVINFO *psClientDevInfo);

IMG_IMPORT PVRSRV_ERROR SetMode(IPHONELCD_DEVINFO		*psDevInfo,
								SYSTEM_ADDR			*psSysBusAddr,
								DISPLAY_MODE_INFO	*psModeInfo);

#ifdef SUPPORT_OEM_FUNCTION
PVRSRV_ERROR OEMFunc(IPHONELCD_DEVINFO	*psDevInfo,
					 IMG_UINT32		ui32CmdID,
					 IMG_BYTE*		pbInData,
					 IMG_UINT32		ui32pbInDataSize,
					 IMG_BYTE*		pbOutData,
					 IMG_UINT32		ui32pbOutDataSize);
#endif

#ifdef BUFFER_OVERLAY
PVRSRV_ERROR CreateSwapchain (	IPHONELCD_DEVINFO			*psDevInfo,
								IMG_UINT32				ui32Flags,
								IMG_UINT32				ui32BufferCount,
								IPHONELCD_SWAP_CHAIN		*psSwapChainOut,
								IMG_UINT32				ui32OEMFlags, 
								DISPLAY_SURF_ATTRIBUTES	*psDstSurfAttrib,
								DISPLAY_SURF_ATTRIBUTES	*psSrcSurfAttrib,
								IMG_UINT32				*pui32SwapChainID);
#else
IMG_IMPORT PVRSRV_ERROR CreateSwapchain (
								IPHONELCD_DEVINFO			*psDevInfo,
								IMG_UINT32				ui32Flags,
								IMG_UINT32				ui32BufferCount,
								IPHONELCD_SWAP_CHAIN		*psSwapChainOut,
								DISPLAY_SURF_ATTRIBUTES	*psDstSurfAttrib,
								DISPLAY_SURF_ATTRIBUTES	*psSrcSurfAttrib,
								IMG_UINT32				*pui32SwapChainID);
#endif 

IMG_IMPORT PVRSRV_ERROR DestroySwapchain ( IPHONELCD_SWAP_CHAIN	*psSwapChain, IMG_BOOL bDestroyCommandQ);

IMG_VOID VSyncFlip(IPHONELCD_DEVINFO	*psDevInfo);

extern IMG_IMPORT IMG_BOOL PVRGetDisplayClassJTable(PVRSRV_DC_KMJTABLE *psJTable);

#ifdef BUFFER_OVERLAY
IMG_VOID FlipOverlay(OIPHONELCD_DEVINFO *psDevInfo, unsigned long ui32DevAddr);
#endif
IMG_VOID Flip(IPHONELCD_DEVINFO *psDevInfo, unsigned long ui32DevAddr);

IMG_VOID FlushQueue( IPHONELCD_DEVINFO *psDevInfo,
					 IMG_VOID (*pfFlipAction)(IMG_VOID *, unsigned long),
					 IMG_VOID * pArg );

IMG_CHAR const * GetClientName(void);

PVRSRV_ERROR OpenPVRServices (IMG_CHAR *szPVRKernelServicesName, IMG_HANDLE *phPVRServices);

PVRSRV_ERROR ClosePVRServices (IMG_HANDLE hPVRServices);

PVRSRV_ERROR GetLibFuncAddr (IMG_HANDLE hExtDrv, IMG_CHAR *szFunctionName, IMG_VOID **ppvFuncAddr);

#ifdef BUFFER_OVERLAY

PVRSRV_ERROR GetOverlayBackBuffer(OMAPLCD_DEVINFO *psDevInfo, IMG_UINT32 uiSize, IMG_UINT32 ui32Num);

#ifdef OVERLAY_DYNAMICBUFFERS

PVRSRV_ERROR ReleaseOverlayBuffers();
PVRSRV_ERROR ReleaseBackBuffers();

#else 

PVRSRV_ERROR ReleaseOverlayBuffers(IMG_SYS_PHYADDR *pFirstAllocatedBufferPhysAddr);

#endif 
#endif 

PVRSRV_ERROR GetBackBuffer(IPHONELCD_DEVINFO *psDevInfo, IMG_UINT32 ui32Num);

PVRSRV_ERROR ReleaseBackBuffer(IPHONELCD_DEVINFO *psDevInfo, IMG_UINT32 ui32Num);

IMG_VOID *AllocKernelMem(IMG_UINT32 ui32Size);

IMG_VOID FreeKernelMem(IMG_VOID *pvMem);

IMG_VOID WriteReg(IMG_VOID *pvAddr, IMG_UINT32 ui32Value);

IMG_UINT32 ReadReg(IMG_VOID *pvAddr);

PVRSRV_ERROR GetSystemSurfaceInfo(IPHONELCD_DEVINFO *psDevInfo);

PVRSRV_ERROR ReleaseSystemSurfaceInfo(IPHONELCD_DEVINFO *psDevInfo);

PVRSRV_ERROR InstallVsyncISR(IPHONELCD_DEVINFO *psDevInfo);

PVRSRV_ERROR UnInstallVsyncISR(IPHONELCD_DEVINFO *psDevInfo);

IMG_BOOL IsPowerDown(IPHONELCD_DEVINFO *psDevInfo);


#if defined (__cplusplus)
}
#endif

#endif 

