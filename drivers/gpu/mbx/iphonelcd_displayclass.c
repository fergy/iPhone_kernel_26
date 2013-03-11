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
   
  
#ifdef BUFFER_OVERLAY
#include <windows.h>
#endif	/*  */
  
#include "img_defs.h"
#include "servicesext.h"
#include "kerneldisplay.h"
  
#include <linux/slab.h>
#include "mbxdisplay.h"
  
#include "iphonelcdif.h"
#include "iphonelcd_int.h"
  
#ifdef BUFFER_OVERLAY
#include "iphone3g_display.h"
#endif	/*  */
  
#define DISPC_CONFIG				  0x444	
#define DISPC_TRANS_COLOR0			  0x454	
#define DISPC_VID2_BA0				  0x54C	
#define DISPC_VID2_BA1				  0x550	
#define DISPC_VID2_POSITION			  0x554	
#define DISPC_VID2_SIZE				  0x558	
#define DISPC_VID2_ATTRIBUTES		  0x55C	
#define DISPC_VID2_FIFO_THRESHOLD	  0x560	
#define DISPC_VID2_ROW_INC			  0x568	
#define DISPC_VID2_PIXEL_INC		  0x56C	
#define DISPC_VID2_PICTURE_SIZE		  0x574	
#define DISPC_VID2_CONV_COEF0		  0x5C0	
#define DISPC_VID2_CONV_COEF1		  0x5C4	
#define DISPC_VID2_CONV_COEF2		  0x5C8	
#define DISPC_VID2_CONV_COEF3		  0x5CC	
#define DISPC_VID2_CONV_COEF4		  0x5D0

static IMG_VOID *gpvAnchor = IMG_NULL;
static IMG_UINT32 gui32DeviceID = 0;
static IMG_HANDLE gPVRServices = IMG_NULL;
static PFN_DC_GET_PVRJTABLE pfnGetPVRJTable = IMG_NULL;
IPHONELCD_DEVINFO * GetAnchorPtr (IMG_VOID) 
{
  return (IPHONELCD_DEVINFO *) gpvAnchor;
}

static IMG_VOID
SetAnchorPtr (IPHONELCD_DEVINFO * psDevInfo) 
{
  gpvAnchor = (IMG_VOID *) psDevInfo;
} 

#ifdef BUFFER_OVERLAY
  IMG_VOID FlipOverlay (IPHONELCD_DEVINFO * psDevInfo,
			unsigned long ui32DevAddr) 
{
  IMG_UINT32 control;
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			     DISPC_VID2_BA0), ui32DevAddr);
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   DISPC_VID2_BA1), ui32DevAddr);
  control =
    ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_CONTROL));
  control |= IPHONE_CONTROL_GOLCD;
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_CONTROL), control);
} 
#endif	/*  */
  IMG_VOID Flip (IPHONELCD_DEVINFO * psDevInfo, unsigned long ui32DevAddr) 
{
  IMG_UINT32 control;
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			     IPHONELCD_GFX_BA0), ui32DevAddr);
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_GFX_BA1), ui32DevAddr);
  control =
    ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_CONTROL));
  control |= IPHONE_CONTROL_GOLCD;
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_CONTROL), control);
} static IMG_BOOL

iphonelcd_ddc_swaptosystem (IMG_HANDLE hPrivateDevice) 
{
  IPHONELCD_DEVINFO * psDevInfo;
  if (hPrivateDevice == IMG_NULL)
    
    {
      return IMG_FALSE;
    }
  psDevInfo = (IPHONELCD_DEVINFO *) hPrivateDevice;
  if (psDevInfo->psSwapChain)
    
    {
      DestroySwapchain (psDevInfo->psSwapChain, IMG_FALSE);
    }
  if (!psDevInfo->bIsPoweredDown)
    Flip (psDevInfo,
	   psDevInfo->sClientDevInfo.sSystemBuffer.sSysAddr.u.sContig.uiAddr);
  return IMG_TRUE;
}

static IMG_BOOL
iphonelcd_ddc_process_vsyncflip (IMG_HANDLE hCmdCookie,
			       IMG_UINT32 ui32DataSize, IMG_VOID * pvData) 
{
  IPHONELCD_PRIVATE_FLIP_CMD * psFlipCmd;
  IPHONELCD_DEVINFO * psDevInfo;
  IPHONELCD_VSYNC_FLIP_ITEM * psFlipItem;
  IPHONELCD_SWAP_CHAIN * psSwapChain;
  psFlipCmd = (IPHONELCD_PRIVATE_FLIP_CMD *) pvData;
  if (psFlipCmd == IMG_NULL || psFlipCmd->ui32CmdSize != ui32DataSize)
    
    {
      return IMG_FALSE;
    }
  psDevInfo = (IPHONELCD_DEVINFO *) psFlipCmd->hDevHandle;
  psSwapChain = psDevInfo->psSwapChain;
  if (IsPowerDown (psDevInfo))
    
    {
      psDevInfo->ui32LastIgoredSurfaceAddr = psFlipCmd->ui32DevAddr;
      psDevInfo->pfnCmdCompleteKM (hCmdCookie);
      return IMG_TRUE;
    }
  if (psFlipCmd->ui32FlipInterval == 0)
    
    {
      
#ifdef BUFFER_OVERLAY
	if (psFlipCmd->bOverlay)
	FlipOverlay (psDevInfo, psFlipCmd->ui32DevAddr);
      
      else
	
#endif	/*  */
	  Flip (psDevInfo, psFlipCmd->ui32DevAddr);
      psDevInfo->pfnCmdCompleteKM (hCmdCookie);
      return IMG_TRUE;
    }
  psFlipItem = &psDevInfo->asVsyncFlips[psDevInfo->ui32InsertIndex];
  if (psFlipItem->bValid == IMG_FALSE)
    
    {
      IMG_UINT32 ui32MaxIndex = psSwapChain->ui32BufferCount - 1;
      if (psDevInfo->ui32InsertIndex == psDevInfo->ui32RemoveIndex)
	
	{
	  Flip (psDevInfo, psFlipCmd->ui32DevAddr);
	  psFlipItem->bFlipped = IMG_TRUE;
	}
      psFlipItem->hCmdComplete = hCmdCookie;
      psFlipItem->ui32DevAddr = psFlipCmd->ui32DevAddr;
      psFlipItem->ui32SwapInterval = psFlipCmd->ui32FlipInterval;
      psFlipItem->bValid = IMG_TRUE;
      psDevInfo->ui32InsertIndex++;
      if (psDevInfo->ui32InsertIndex >= ui32MaxIndex)
	psDevInfo->ui32InsertIndex = 0;
      return IMG_TRUE;
    }
  return IMG_FALSE;
}

IMG_VOID VSyncFlip (IPHONELCD_DEVINFO * psDevInfo) 
{
  IMG_UINT32 ui32IntMask;
  IPHONELCD_VSYNC_FLIP_ITEM * psFlipItem;
  IMG_UINT32 ui32MaxIndex;
  ui32IntMask =
    ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			   IPHONELCD_IRQENABLE));
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			    IPHONELCD_IRQENABLE), IPHONELCD_INTMASK_OFF);
  if (!psDevInfo->psSwapChain)
    
    {
      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				IPHONELCD_IRQENABLE), ui32IntMask);
      return;
    }
  psFlipItem = &psDevInfo->asVsyncFlips[psDevInfo->ui32RemoveIndex];
  ui32MaxIndex = gui32BufferCount - 1;
  while (psFlipItem->bValid)
    
    {
      if (psFlipItem->bFlipped)
	
	{
	  if (!psFlipItem->bCmdCompleted)
	    
	    {
	      psDevInfo->pfnCmdCompleteKM (psFlipItem->hCmdComplete);
	      psFlipItem->bCmdCompleted = IMG_TRUE;
	    }
	  psFlipItem->ui32SwapInterval--;
	  if (psFlipItem->ui32SwapInterval <= 0)
	    
	    {
	      psDevInfo->ui32RemoveIndex++;
	      if (psDevInfo->ui32RemoveIndex == ui32MaxIndex)
		
		{
		  psDevInfo->ui32RemoveIndex = 0;
		}
	      psFlipItem->bCmdCompleted = IMG_FALSE;
	      psFlipItem->bFlipped = IMG_FALSE;
	      psFlipItem->bValid = IMG_FALSE;
	    }
	  
	  else
	    
	    {
	      break;
	    }
	}
      
      else
	
	{
	  Flip (psDevInfo, psFlipItem->ui32DevAddr);
	  psFlipItem->bFlipped = IMG_TRUE;
	  break;
	}
      psFlipItem = &psDevInfo->asVsyncFlips[psDevInfo->ui32RemoveIndex];
    }
  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			     IPHONELCD_IRQENABLE), ui32IntMask);
}

PVRSRV_ERROR InitMain (IMG_VOID * pvKernelPage) 
{
  IPHONELCD_DEVINFO * psDevInfo;
  psDevInfo = GetAnchorPtr ();
  if (psDevInfo == IMG_NULL)
    
    {
      PVRSRV_DC_KMJTABLE * psJTable;
      PFN_CMD_PROC pfnCmdProcList[IPHONELCD_COMMAND_COUNT];
      IMG_UINT32 aui32SyncCountList[IPHONELCD_COMMAND_COUNT][2];
      IMG_UINT32 i;
      psDevInfo = pvKernelPage;
      if (!psDevInfo)
	
	{
	  return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
      SetAnchorPtr ((IMG_VOID *) psDevInfo);
      psDevInfo->ui32RefCount = 0;
      psDevInfo->sClientDevInfo.hDevInfo = (IMG_HANDLE) psDevInfo;
      if (GetSystemSurfaceInfo (psDevInfo) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      if (OpenPVRServices ("PVRServices", &psDevInfo->hPVRServices) !=
	    PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      if (GetLibFuncAddr
	    (psDevInfo->hPVRServices, "PVRGetDisplayClassJTable",
	     (IMG_VOID **) & pfnGetPVRJTable) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      if (pfnGetPVRJTable (&psDevInfo->sPVRDCJTable) == IMG_FALSE)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      psJTable = &psDevInfo->sPVRDCJTable;
      psDevInfo->sClientDevInfo.ui32DeviceID = gui32DeviceID;
      pfnCmdProcList[IPHONELCD_FLIP_COMMAND] = iphonelcd_ddc_process_vsyncflip;
      aui32SyncCountList[IPHONELCD_FLIP_COMMAND][0] = 0;
      aui32SyncCountList[IPHONELCD_FLIP_COMMAND][1] = 2;
      if (psJTable->
		 pfnPVRSRVRegisterCmdProcList (psDevInfo->sClientDevInfo.
					       ui32DeviceID,
					       &pfnCmdProcList[0],
					       aui32SyncCountList,
					       IPHONELCD_COMMAND_COUNT) !=
		 PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_CANT_REGISTER_CALLBACK;
	}
      if (psJTable->
	      pfnPVRSRVGetLISRCommandCallback (&psDevInfo->
					       pfnCmdCompleteKM) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      psDevInfo->sClientDevInfo.ui32NumFormats = 1;
      psDevInfo->sClientDevInfo.asDisplayFormatList[0].ui32DimsCount = 1;
      psDevInfo->sClientDevInfo.asDisplayFormatList[0].pixelformat =
	psDevInfo->sFBInfo.ePixelFormat;
      psDevInfo->sClientDevInfo.asDisplayFormatList[0].asDimsList[0].
	ui32ByteStride = psDevInfo->sFBInfo.ui32ByteStride;
      psDevInfo->sClientDevInfo.asDisplayFormatList[0].asDimsList[0].
	ui32Width = psDevInfo->sFBInfo.ui32Width;
      psDevInfo->sClientDevInfo.asDisplayFormatList[0].asDimsList[0].
	ui32Height = psDevInfo->sFBInfo.ui32Height;
      psDevInfo->sClientDevInfo.sSysFormat =
	psDevInfo->sClientDevInfo.asDisplayFormatList[0];
      psDevInfo->sClientDevInfo.sSystemBuffer.psSyncObj =
	psDevInfo->sPVRDCJTable.pfnPVRSRVCreateSyncObj ();
      psDevInfo->sClientDevInfo.sSystemBuffer.sSysAddr.ui32PageCount = 0;
      psDevInfo->sClientDevInfo.sSystemBuffer.sSysAddr.u.sContig =
	psDevInfo->sFBInfo.sSysAddr.u.sContig;
      psDevInfo->sClientDevInfo.sDisplayInfo.ui32MinSwapInterval = 0;
      psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapInterval = 3;
      psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapChains = 1;
      psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapChainBuffers = 1;
      psDevInfo->sBackBufferFormat[0] =
	psDevInfo->sClientDevInfo.asDisplayFormatList[0];
      for (i = 0; i < IPHONELCD_MAX_BACKBUFFERS; i++)
	
	{
	  psDevInfo->asBackBuffers[i].sSysAddr.ui32PageCount = 0;
	  
#ifdef OVERLAY_DYNAMICBUFFERS
	    psDevInfo->asBackBuffers[i].sSysAddr.u.sContig.uiAddr = 0;
	  psDevInfo->asBackBuffers[i].sCPUVAddr = 0;
	  psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapChainBuffers++;
	  
#else	/*  */
	    if (GetBackBuffer (psDevInfo, i) == PVRSRV_OK)
	    
	    {
	      psDevInfo->sClientDevInfo.sDisplayInfo.
		ui32MaxSwapChainBuffers++;
	    }
	  
	  else
	    
	    {
	      break;
	    }
	  
#endif	/*  */
	    psDevInfo->asBackBuffers[i].psSyncObj =
	    psDevInfo->sPVRDCJTable.pfnPVRSRVCreateSyncObj ();
	  psDevInfo->asVsyncFlips[i].bValid = IMG_FALSE;
	  psDevInfo->asVsyncFlips[i].bFlipped = IMG_FALSE;
	  psDevInfo->asVsyncFlips[i].bCmdCompleted = IMG_FALSE;
	}
      psDevInfo->ui32InsertIndex = 0;
      psDevInfo->ui32RemoveIndex = 0;
      psDevInfo->psSwapChain = IMG_NULL;
      
#ifdef BUFFER_OVERLAY
	psDevInfo->psOvlSwapChain = IMG_NULL;
      
#endif	/*  */
	if (InstallVsyncISR (psDevInfo) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_INIT_FAILURE;
	}
      psDevInfo->bIsPoweredDown = IMG_FALSE;
      psDevInfo->ui32LastIgoredSurfaceAddr = IMG_FALSE;
      psDevInfo->ui32LastQueuedSurfaceAddr = IMG_FALSE;
    }
  psDevInfo->ui32FlipTokenUser = 0;
  psDevInfo->ui32FlipTokenKernel = 0;
  psDevInfo->ui32RefCount++;
  return PVRSRV_OK;
}

PVRSRV_ERROR Init (IMG_VOID) 
{
  PVRSRV_DC_KMJTABLE sPVRDCJTable;
  IMG_CHAR const *pszClientDrvName = GetClientName ();
  if (OpenPVRServices ("PVRServices", &gPVRServices) != PVRSRV_OK)
    
    {
      return PVRSRV_ERROR_INIT_FAILURE;
    }
  if (GetLibFuncAddr
	(gPVRServices, "PVRGetDisplayClassJTable",
	 (IMG_VOID **) & pfnGetPVRJTable) != PVRSRV_OK)
    
    {
      return PVRSRV_ERROR_INIT_FAILURE;
    }
  if (pfnGetPVRJTable (&sPVRDCJTable) == IMG_FALSE)
    
    {
      return PVRSRV_ERROR_INIT_FAILURE;
    }
  if (sPVRDCJTable.
	 pfnPVRSRVRegisterDisplayClassDevice (iphonelcd_ddc_swaptosystem,
					      IMG_NULL,
					      (IMG_CHAR *) pszClientDrvName,
					      &gui32DeviceID) != PVRSRV_OK)
    
    {
      return PVRSRV_ERROR_INIT_FAILURE;
    }
  return PVRSRV_OK;
}

PVRSRV_ERROR Deinit (IMG_VOID) 
{
  PVRSRV_DC_KMJTABLE sPVRDCJTable;
  if (pfnGetPVRJTable (&sPVRDCJTable) == IMG_FALSE)
    
    {
      return PVRSRV_ERROR_INIT_FAILURE;
    }
  if (sPVRDCJTable.pfnPVRSRVRemoveDisplayClassDevice (gui32DeviceID) !=
	 PVRSRV_OK)
    
    {
      return PVRSRV_ERROR_GENERIC;
    }
  if (ClosePVRServices (gPVRServices) != PVRSRV_OK)
    
    {
      return PVRSRV_ERROR_GENERIC;
    }
  return PVRSRV_OK;
}

PVRSRV_ERROR DeinitMain (IMG_VOID) 
{
  IPHONELCD_DEVINFO * psDevInfo;
  psDevInfo = GetAnchorPtr ();
  if (psDevInfo == IMG_NULL)
    
    {
      return PVRSRV_ERROR_GENERIC;
    }
  psDevInfo->ui32RefCount--;
  if (psDevInfo->ui32RefCount != 0)
    
    {
      printk (KERN_ALERT "DeinitMain refcount is:%lu\n",
	       psDevInfo->ui32RefCount);
    }
  
  else
    
    {
      PVRSRV_DC_KMJTABLE * psJTable = &psDevInfo->sPVRDCJTable;
      IMG_UINT32 i;
      
#ifndef OVERLAY_DYNAMICBUFFERS
	IMG_UINT32 ui32NumBuffers;
      ui32NumBuffers =
	psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapChainBuffers;
      
#endif	/*  */
	if (UnInstallVsyncISR (psDevInfo) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_GENERIC;
	}
      
#ifdef OVERLAY_DYNAMICBUFFERS
	for (i = 0; i < IPHONELCD_MAX_BACKBUFFERS; i++)
	
	{
	  if (psJTable->
	       pfnPVRSRVDestroySyncObj (psDevInfo->asBackBuffers[i].
					psSyncObj) != PVRSRV_OK)
	    
	    {
	      return PVRSRV_ERROR_GENERIC;
	    }
	}
      
#else	/*  */
	for (i = 0; i < ui32NumBuffers - 1; i++)
	
	{
	  if (ReleaseBackBuffer (psDevInfo, i) != PVRSRV_OK)
	    
	    {
	      return PVRSRV_ERROR_GENERIC;
	    }
	  if (psJTable->
		pfnPVRSRVDestroySyncObj (psDevInfo->asBackBuffers[i].
					 psSyncObj) != PVRSRV_OK)
	    
	    {
	      return PVRSRV_ERROR_GENERIC;
	    }
	}
      
#endif	/*  */
	if (psJTable->
	      pfnPVRSRVDestroySyncObj (psDevInfo->sClientDevInfo.
				       sSystemBuffer.psSyncObj) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_GENERIC;
	}
      if (psJTable->
	     pfnPVRSRVRemoveCmdProcList (psDevInfo->sClientDevInfo.
					 ui32DeviceID,
					 IPHONELCD_COMMAND_COUNT) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_GENERIC;
	}
      if (ReleaseSystemSurfaceInfo (psDevInfo) != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_GENERIC;
	}
      SetAnchorPtr (IMG_NULL);
    }
  return PVRSRV_OK;
}

IMG_EXPORT 
  PVRSRV_ERROR GetClientInfo (IPHONELCD_CLIENT_DEVINFO * psClientDevInfo) 
{
  IPHONELCD_DEVINFO * psDevInfo;
  psDevInfo = GetAnchorPtr ();
  if (psDevInfo == IMG_NULL)
    
    {
      return PVRSRV_ERROR_GENERIC;
    }
  *psClientDevInfo = psDevInfo->sClientDevInfo;
  return PVRSRV_OK;
}


#if 0
static SYSTEM_ADDR *
iphonelcd_ddc_mapphystosysaddr (IMG_UINT32 ui32DevPhysAddr) 
{
  return IMG_NULL;
}

static IMG_VOID
iphonelcd_ddc_setmode (DISPLAY_FORMAT * psFormat, DISPLAY_DIMS * psDims,
		     SYSTEM_ADDR * psSysAddr) 
{
} static IPHONELCD_BUFFER *

iphonelcd_ddc_getsystembuffer (IMG_VOID) 
{
  return IMG_NULL;
}


#endif	/*  */
  IMG_EXPORT 
  PVRSRV_ERROR SetMode (IPHONELCD_DEVINFO * psDevInfo,
			SYSTEM_ADDR * psSysBusAddr,
			DISPLAY_MODE_INFO * psModeInfo) 
{
  PVR_UNREFERENCED_PARAMETER (psDevInfo);
  PVR_UNREFERENCED_PARAMETER (psSysBusAddr);
  PVR_UNREFERENCED_PARAMETER (psModeInfo);
  return PVRSRV_OK;
}

void
FlushQueue (IPHONELCD_DEVINFO * psDevInfo,
	    IMG_VOID (*pfFlipAction) (IMG_VOID *, unsigned long),
	    IMG_VOID * pArg) 
{
  IPHONELCD_VSYNC_FLIP_ITEM * psFlipItem;
  IMG_UINT32 ui32MaxIndex;
  psFlipItem = &psDevInfo->asVsyncFlips[psDevInfo->ui32RemoveIndex];
  ui32MaxIndex = psDevInfo->psSwapChain->ui32BufferCount - 1;
  while (psFlipItem->bValid)
    
    {
      if (psFlipItem->bFlipped == IMG_FALSE)
	
	{
	  pfFlipAction (pArg, psFlipItem->ui32DevAddr);
	}
      if (psFlipItem->bCmdCompleted == IMG_FALSE)
	
	{
	  psDevInfo->pfnCmdCompleteKM (psFlipItem->hCmdComplete);
	}
      psDevInfo->ui32RemoveIndex++;
      if (psDevInfo->ui32RemoveIndex == ui32MaxIndex)
	
	{
	  psDevInfo->ui32RemoveIndex = 0;
	}
      psFlipItem->bFlipped = IMG_FALSE;
      psFlipItem->bCmdCompleted = IMG_FALSE;
      psFlipItem->bValid = IMG_FALSE;
      psFlipItem = &psDevInfo->asVsyncFlips[psDevInfo->ui32RemoveIndex];
    }
  psDevInfo->ui32InsertIndex = 0;
  psDevInfo->ui32RemoveIndex = 0;
}


#ifdef SUPPORT_OEM_FUNCTION
  PVRSRV_ERROR OEMFunc (IPHONELCD_DEVINFO * psDevInfo, IMG_UINT32 ui32CmdID,
			 IMG_BYTE * pbInData, IMG_UINT32 ui32pbInDataSize,
			 IMG_BYTE * pbOutData,
			 IMG_UINT32 ui32pbOutDataSize) 
{
  PVRSRV_ERROR eError = PVRSRV_ERROR_GENERIC;
  switch (ui32CmdID)
    
    {
    case OEM_FUNC_GET_POWER_STATE:
      
      {
	if (psDevInfo)
	  
	  {
	    if (pbOutData != IMG_NULL
		 && (ui32pbOutDataSize == sizeof (IMG_UINT32)))
	      
	      {
		*((IMG_UINT32 *) pbOutData) = psDevInfo->ui32CurrentDx;
		eError = PVRSRV_OK;
	      }
	  }
	break;
      }
    case OEM_FUNC_POWER_CAPABILITIES:
      
      {
	break;
      }
    case OEM_FUNC_CONTRAST_GET:
    case OEM_FUNC_CONTRAST_SET:
    case OEM_FUNC_CONTRAST_INCREASE:
    case OEM_FUNC_CONTRAST_DECREASE:
    case OEM_FUNC_CONTRAST_DEFAULT:
    case OEM_FUNC_CONTRAST_MAX:
      
      {
	if (ui32pbInDataSize == sizeof (int))
	  
	  {
	    int i32Contrast = *((int *) pbInData);
	    if (ui32pbOutDataSize == sizeof (int))
	      
	      {
		*((int *) pbOutData) = i32Contrast;
		eError = PVRSRV_OK;
	      }
	  }
	break;
      }
      
#ifdef BUFFER_OVERLAY
    case OEM_FUNC_OVERLAY:
      
      {
	IMG_UINT32 ui32Control;
	IMG_UINT32 ui32Attributes = 0;
	OVERLAY_INFO * pOverlayInfo = (OVERLAY_INFO *) pbInData;
//FIXME!!!
	IpHoNe_SetPower (0, psDevInfo);
//////////
	switch (pOverlayInfo->eOverlayCmd)
	  
	  {
	  case OC_NONE:
	    RETAILMSG (TRUE, (L"\t\t\t\tNONE OVERLAY COMMAND"));
	    break;
	  case OC_SHOW:
	    
	    {
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					pvRegs + DISPC_VID2_BA0),
			  psDevInfo->asOvlBackBuffers[0].sSysAddr.u.sContig.
			  uiAddr +
			  (2 *
			   (pOverlayInfo->nPosOnOvlY *
			    pOverlayInfo->nSurfaceWidth +
			    pOverlayInfo->nPosOnOvlX)));
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
				       pvRegs + DISPC_VID2_BA1),
			 psDevInfo->asOvlBackBuffers[0].sSysAddr.u.sContig.
			 uiAddr +
			 (2 *
			  (pOverlayInfo->nPosOnOvlY *
			   pOverlayInfo->nSurfaceWidth +
			   pOverlayInfo->nPosOnOvlX)));
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
				       pvRegs + DISPC_VID2_SIZE),
			 ((((pOverlayInfo->nHeight) -
			    1) << 16) | ((pOverlayInfo->nWidth) - 1)));
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
				       pvRegs + DISPC_VID2_PICTURE_SIZE),
			 ((((pOverlayInfo->nHeight) -
			    1) << 16) | ((pOverlayInfo->nWidth) - 1)));
	      ui32Attributes = (0x0 << 12);
	      ui32Attributes |= (0x1);
	      switch (pOverlayInfo->ePixelFormat)
		
		{
		case PVRSRV_PIXEL_FORMAT_RGB565:
		  ui32Attributes |= (0x0 << 9) | (0x6 << 1);
		  break;
		case PVRSRV_PIXEL_FORMAT_YUYV:
		  ui32Attributes |= (0x0 << 11) | (0x1 << 9) | (0xA << 1);
		  break;
		case PVRSRV_PIXEL_FORMAT_UYVY:
		  ui32Attributes |= (0x0 << 11) | (0x1 << 9) | (0xB << 1);
		  break;
		default:
		  RETAILMSG (TRUE, (L"\t\t\t\tUNKNOWN OVERLAY FORMAT"));
		  break;
		}
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					pvRegs + DISPC_VID2_ATTRIBUTES),
			  ui32Attributes);
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					pvRegs + DISPC_VID2_FIFO_THRESHOLD),
			  ((252 << 16) | (192)));
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					pvRegs + DISPC_VID2_ROW_INC),
			  2 * (pOverlayInfo->nSurfaceWidth -
			       pOverlayInfo->nWidth) + 1);
	      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
				       pvRegs + DISPC_VID2_PIXEL_INC), 1);
	      if (pOverlayInfo->bCKeyOn)
		
		{
		  RETAILMSG (TRUE,
			      (L"Color key = %d", pOverlayInfo->ui16CKColor));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					     pvRegs + DISPC_TRANS_COLOR0),
			       ((0x0 << 16) | (pOverlayInfo->ui16CKColor)));
		  ui32Control =
		    ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_CONFIG));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_CONFIG),
			     ui32Control | ((0x1) << 11) | ((0x1) << 10));
		}
	      
	      else
		
		{
		  ui32Control =
		    ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_CONFIG));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_CONFIG),
			     ui32Control | ((0x0) << 10));
		}
	      if (pOverlayInfo->ePixelFormat != PVRSRV_PIXEL_FORMAT_RGB565)
		
		{
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					    pvRegs + DISPC_VID2_CONV_COEF0),
			      (409 << 16) | (298));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_VID2_CONV_COEF1),
			     (298 << 16) | (0));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_VID2_CONV_COEF2),
			     (((-100) & 0x07FF) << 16) | ((-208) & 0x07FF));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_VID2_CONV_COEF3),
			     (0 << 16) | (298));
		  WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.
					   pvRegs + DISPC_VID2_CONV_COEF4),
			     (0 << 16) | (517));
		}
	    }
	  case OC_SETPOS:
	    WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				     DISPC_VID2_POSITION),
		       DISPC_VID_POS_VIDPOSY (pOverlayInfo->
					      nPosOnPriY) |
		       DISPC_VID_POS_VIDPOSX (pOverlayInfo->nPosOnPriX));
	    break;
	  case OC_HIDE:
	    ui32Attributes = (0x0);
	    WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				     DISPC_VID2_ATTRIBUTES), ui32Attributes);
	    break;
	  default:
	    DEBUGMSG (TRUE, (L"\t\t\t\tUNKNOWN OVERLAY COMMAND"));
	    break;
	  }
	ui32Control =
	  ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				 IPHONELCD_CONTROL));
	WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				   IPHONELCD_CONTROL),
		     ui32Control | IPHONE_CONTROL_LCDENABLE |
		     IPHONE_CONTROL_GOLCD);
	while (ReadReg
		 ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				IPHONELCD_CONTROL)) & IPHONE_CONTROL_GOLCD);
	eError = PVRSRV_OK;
      }
      break;
      
#endif	/*  */
    }
  return eError;
}


#endif	/*  */
  
#ifdef BUFFER_OVERLAY
  IMG_EXPORT 
  PVRSRV_ERROR CreateSwapchain (IPHONELCD_DEVINFO * psDevInfo,
				IMG_UINT32 ui32Flags,
				IMG_UINT32 ui32BufferCount,
				IPHONELCD_SWAP_CHAIN * psSwapChainOut,
				IMG_UINT32 ui32OEMFlags,
				DISPLAY_SURF_ATTRIBUTES * psDstSurfAttrib,
				DISPLAY_SURF_ATTRIBUTES * psSrcSurfAttrib,
				IMG_UINT32 * pui32SwapChainID) 
#else	/*  */
  IMG_EXPORT 
  PVRSRV_ERROR CreateSwapchain (IPHONELCD_DEVINFO * psDevInfo,
				IMG_UINT32 ui32Flags,
				IMG_UINT32 ui32BufferCount,
				IPHONELCD_SWAP_CHAIN * psSwapChainOut,
				DISPLAY_SURF_ATTRIBUTES * psDstSurfAttrib,
				DISPLAY_SURF_ATTRIBUTES * psSrcSurfAttrib,
				IMG_UINT32 * pui32SwapChainID) 
#endif	/*  */
{
  PVRSRV_ERROR eError;
  IPHONELCD_SWAP_CHAIN * psSwapChain;
  IMG_UINT32 i;
  
#ifdef BUFFER_OVERLAY
    IMG_BOOL bOverlayRequested = IMG_FALSE;
  
#endif	/*  */
    if ((psDevInfo == IMG_NULL) 
	  ||(psDstSurfAttrib == IMG_NULL) 
	  ||(psSrcSurfAttrib == IMG_NULL) 
	  ||(psSwapChainOut == IMG_NULL)  ||(ui32BufferCount == 0))
    
    {
      return PVRSRV_ERROR_INVALID_PARAMS;
    }
  
#ifdef BUFFER_OVERLAY
    if ((ui32OEMFlags & SWAPCHAIN_OVERLAY) != 0)
    bOverlayRequested = IMG_TRUE;
  
#endif	/*  */
    if ((ui32Flags & PVRSRV_CREATE_SWAPCHAIN_QUERY) == 0)
    
    {
      
#ifdef BUFFER_OVERLAY
	if (bOverlayRequested)
	
	{
	  if (psDevInfo->psOvlSwapChain)
	    
	    {
	      return PVRSRV_ERROR_FLIP_CHAIN_EXISTS;
	    }
	}
      
      else
	
#endif	/*  */
	if (psDevInfo->psSwapChain)
	
	{
	  return PVRSRV_ERROR_FLIP_CHAIN_EXISTS;
	}
      if (ui32BufferCount >
	    psDevInfo->sClientDevInfo.sDisplayInfo.ui32MaxSwapChainBuffers)
	
	{
	  return PVRSRV_ERROR_INVALID_PARAMS;
	}
      psSwapChain = AllocKernelMem (sizeof (IPHONELCD_SWAP_CHAIN));
      if (psSwapChain == IMG_NULL)
	
	{
	  return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
      psSwapChain->hSwapChain = (IMG_HANDLE) psSwapChain;
      psSwapChain->hDevice = (IMG_HANDLE) psDevInfo;
      
#ifdef BUFFER_OVERLAY
	psSwapChain->bOverlay = IMG_FALSE;
      
#endif	/*  */
	eError =
	psDevInfo->sPVRDCJTable.pfnPVRSRVCreateCommandQueue (512,
							     &psSwapChain->
							     psQueueKM);
      if (eError != PVRSRV_OK)
	
	{
	  return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
      if (
#ifdef BUFFER_OVERLAY
	     bOverlayRequested || (
#endif	/*  */
				    (psDevInfo->sClientDevInfo.sSysFormat.
				     asDimsList[0].ui32Width ==
				     psDstSurfAttrib->sDims.
				     ui32Width)  &&(psDevInfo->
						     sClientDevInfo.
						     sSysFormat.asDimsList[0].
						     ui32Height ==
						     psDstSurfAttrib->sDims.
						     ui32Height) 
				    &&(psDevInfo->sClientDevInfo.sSysFormat.
				       pixelformat ==
				       psDstSurfAttrib->pixelformat) 
#ifdef BUFFER_OVERLAY
	     ) 
#endif	/*  */
	)
	
	{
	  for (i = 0; i < ui32BufferCount; i++)
	    
	    {
	      
#ifdef BUFFER_OVERLAY
		if (bOverlayRequested)
		
		{
		  if (GetOverlayBackBuffer
		       (psDevInfo,
			psDstSurfAttrib->sDims.ui32ByteStride *
			psDstSurfAttrib->sDims.ui32Height, i) == PVRSRV_OK)
		    
		    {
		      psDevInfo->asOvlBackBuffers[i].psSyncObj =
			psDevInfo->sPVRDCJTable.pfnPVRSRVCreateSyncObj ();
		      psSwapChain->asBufferList[i] =
			psDevInfo->asOvlBackBuffers[i];
		    }
		  
		  else
		    
		    {
		      IMG_UINT32 j;
		      for (j = 0; j < i; j++)
			
			{
			  psDevInfo->sPVRDCJTable.
			    pfnPVRSRVDestroySyncObj (psDevInfo->
						     asOvlBackBuffers[j].
						     psSyncObj);
			}
		      if (i > 0)
			
#ifdef OVERLAY_DYNAMICBUFFERS
			  ReleaseOverlayBuffers ();
		      
#else	/*  */
			  ReleaseOverlayBuffers (&psSwapChain->
						 asBufferList[0].sSysAddr.u.
						 sContig);
		      
#endif	/*  */
			FreeKernelMem (psSwapChain);
		      return PVRSRV_ERROR_INVALID_PARAMS;
		    }
		}
	      
	      else
		
#endif	/*  */
		if (i == 0)
		
		{
		  psSwapChain->asBufferList[i] =
		    psDevInfo->sClientDevInfo.sSystemBuffer;
		}
	      
	      else
		
		{
		  
#ifdef OVERLAY_DYNAMICBUFFERS
		    if (GetBackBuffer (psDevInfo, i - 1) != PVRSRV_OK)
		    
		    {
		      ReleaseBackBuffers ();
		      return PVRSRV_ERROR_INVALID_PARAMS;
		    }
		  
#endif	/*  */
		    psSwapChain->asBufferList[i] =
		    psDevInfo->asBackBuffers[i - 1];
		}
	      psSwapChain->asBufferList[i].hSwapChain =
		(IMG_HANDLE) psSwapChain;
	    }
	  for (i = 0; i < ui32BufferCount - 1; i++)
	    
	    {
	      psSwapChain->asBufferList[i].psNext =
		&psSwapChain->asBufferList[i + 1];
	    }
	  psSwapChain->asBufferList[i].psNext =
	    &psSwapChain->asBufferList[0];
	  psSwapChain->ui32BufferCount = ui32BufferCount;
	  psSwapChain->sSystemBuffer =
	    psDevInfo->sClientDevInfo.sSystemBuffer;
	  psSrcSurfAttrib->sDims.ui32ByteStride =
	    psDevInfo->sClientDevInfo.sSysFormat.asDimsList[0].ui32ByteStride;
	}
      
      else
	
	{
	  return PVRSRV_ERROR_INVALID_PARAMS;
	}
      if (PVRSRV_CREATE_SWAPCHAIN_SHARED & ui32Flags)
	
	{
	  psSwapChain->ui32SwapChainID = 0;
	  *pui32SwapChainID = psSwapChain->ui32SwapChainID;
	}
      
      else
	
	{
	  psSwapChain->ui32SwapChainID = (IMG_UINT32) - 1;
	}
      
#ifdef BUFFER_OVERLAY
	if (bOverlayRequested)
	
	{
	  psDevInfo->psOvlSwapChain = psSwapChain;
	  psSwapChain->bOverlay = IMG_TRUE;
	}
      
      else
	
#endif	/*  */
	  psDevInfo->psSwapChain = psSwapChain;
      *psSwapChainOut = *psSwapChain;
    }
  
  else
    
    {
      if (*pui32SwapChainID == psDevInfo->psSwapChain->ui32SwapChainID)
	
	{
	  *psSwapChainOut = *psDevInfo->psSwapChain;
	}
      
      else
	
	{
	  return PVRSRV_ERROR_INVALID_PARAMS;
	}
    }
  return PVRSRV_OK;
}

IMG_EXPORT 
  PVRSRV_ERROR DestroySwapchain (IPHONELCD_SWAP_CHAIN * psSwapChain,
				 IMG_BOOL bDestroyCommandQ) 
{
  IMG_UINT32 ui32IntMask = 0;
  IPHONELCD_DEVINFO * psDevInfo;
  if (psSwapChain == IMG_NULL)
    
    {
      return PVRSRV_ERROR_INVALID_PARAMS;
    }
  psDevInfo = (IPHONELCD_DEVINFO *) psSwapChain->hDevice;
  if (!psDevInfo->bIsPoweredDown)
    
    {
      ui32IntMask =
	ReadReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			       IPHONELCD_IRQENABLE));
      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
				 IPHONELCD_IRQENABLE), IPHONELCD_INTMASK_OFF);
    }
  
#ifdef BUFFER_OVERLAY
    if (psSwapChain->bOverlay)
    
    {
      IMG_UINT32 i;
      
#ifdef OVERLAY_DYNAMICBUFFERS
	ReleaseOverlayBuffers ();
      
#else	/*  */
	ReleaseOverlayBuffers (&psSwapChain->asBufferList[0].sSysAddr.u.
				sContig);
      
#endif	/*  */
	for (i = 0; i < psSwapChain->ui32BufferCount; i++)
	psDevInfo->sPVRDCJTable.pfnPVRSRVDestroySyncObj (psDevInfo->
							  asOvlBackBuffers[i].
							  psSyncObj);
      psDevInfo->psOvlSwapChain = IMG_NULL;
    }
  
  else
    
    {
      
#endif	/*  */
	FlushQueue (psDevInfo,
		     (IMG_VOID (*)(IMG_VOID *, unsigned long)) Flip,
		     psDevInfo);
      psDevInfo->psSwapChain = IMG_NULL;
      
#ifdef OVERLAY_DYNAMICBUFFERS
	ReleaseBackBuffers ();
      
#endif	/*  */
	
#ifdef BUFFER_OVERLAY
    } 
#endif	/*  */
    FreeKernelMem (psSwapChain);
  
#if 1 
    if (!psDevInfo->bIsPoweredDown)
    
    {
      WriteReg ((IMG_VOID *) ((IMG_UINT8 *) psDevInfo->sFBInfo.pvRegs +
			       IPHONELCD_IRQENABLE), ui32IntMask);
    }
  
#endif	/*  */
    return PVRSRV_OK;
}


