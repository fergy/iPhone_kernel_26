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
#include <linux/config.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
   #include <linux/modversions.h>
   #define MODVERSIONS
#endif
#endif

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/version.h>

#include "services.h"
#include "mbxaccess_mbxsysdata.h"
#include "mbxaccess_mbxmem.h"
#include "pvr_debug.h"
#include "kerneldisplay.h"
#include "kernelbuffer.h"
#include "jumptable_wrappers.h"
#include "malloc_debug.h"


extern IMG_BOOL PVRGetDisplayClassJTable (PVRSRV_DC_KMJTABLE *psJTable);
extern IMG_BOOL PVRGetBufferClassJTable (PVRSRV_BC_KMJTABLE *psJTable);
extern IMG_VOID VSyncCommandComplete(IMG_HANDLE);
EXPORT_SYMBOL(PVRGetDisplayClassJTable);
EXPORT_SYMBOL(PVRGetBufferClassJTable);


#if defined(SUPPORT_INACTIVITY_TIMER_POWER_WRAP)
PVRSRV_ERROR DisplayClassPowerWrapCreateCommandQueueKM(IMG_UINT32 ui32QueueSize, PVRSRV_QUEUE_INFO **ppsQueueInfo)
{
	return PVRSRV_OK;
}

PVRSRV_ERROR DisplayClassPowerWrapDestroyCommandQueueKM(PVRSRV_QUEUE_INFO* psQueueInfo)
{
	return PVRSRV_OK;
}

PVRSRV_ERROR BufferClassPowerWrapCreateCommandQueueKM(IMG_UINT32 ui32QueueSize, PVRSRV_QUEUE_INFO **ppsQueueInfo)
{
	return PVRSRV_OK;
}

PVRSRV_ERROR BufferClassPowerWrapDestroyCommandQueueKM(PVRSRV_QUEUE_INFO* psQueueInfo)
{
	return PVRSRV_OK;
}
#endif

IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVCreateCommandQueueKM(IMG_UINT32 ui32QueueSize, PVRSRV_QUEUE_INFO **ppsQueueInfo)
{
	PVR_UNREFERENCED_PARAMETER(ui32QueueSize);
	PVR_UNREFERENCED_PARAMETER(ppsQueueInfo);
	return PVRSRV_OK;
}


IMG_EXPORT PVRSRV_ERROR IMG_CALLCONV PVRSRVDestroyCommandQueueKM (PVRSRV_QUEUE_INFO *psQueueInfo)
{
	PVR_UNREFERENCED_PARAMETER(psQueueInfo);
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_RegisterDisplayClassDevice (PFN_SWAP_TO_SYSTEM pfnKSwapToSystem,
												IMG_HANDLE hSysSwapData,
												IMG_CHAR *pszClientDrvName,
												IMG_UINT32 *pui32DeviceID )
{
	PVRSRV_DISPLAYCLASS_INFO *psDisplayClassInfo = NULL;
	PVRSRV_DEVICE_NODE       *psDeviceNode = NULL;
	PVRSRV_ERROR              eError = PVRSRV_OK;
	PVR_UNREFERENCED_PARAMETER(pfnKSwapToSystem);
	PVR_UNREFERENCED_PARAMETER(hSysSwapData);
	
	psDisplayClassInfo = KMALLOC(sizeof(PVRSRV_DISPLAYCLASS_INFO), GFP_KERNEL);
	if(!(psDisplayClassInfo))
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to alloc memory for psDisplayClassInfo"));
		return (PVRSRV_ERROR_OUT_OF_MEMORY);
	}
	memset(psDisplayClassInfo, 0, (size_t)sizeof(PVRSRV_DISPLAYCLASS_INFO));

	
	strcpy(psDisplayClassInfo->szClientDrvName, pszClientDrvName);
	psDisplayClassInfo->pfnKSwapToSystem = IMG_NULL;
	psDisplayClassInfo->hSysSwapData = IMG_NULL;
	psDisplayClassInfo->DeviceClass = PVRSRV_DEVICE_CLASS_DISPLAY;
	psDisplayClassInfo->psDCInfoKM = psDisplayClassInfo;

	
	eError = MBXDeviceNodeCreate(&psDeviceNode);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to alloc memory for DeviceNode"));
		KFREE(psDisplayClassInfo);
		return eError;
	}

	psDeviceNode->pvDevice = (IMG_VOID*)psDisplayClassInfo;
	psDeviceNode->ui32pvDeviceSize = sizeof(PVRSRV_DISPLAYCLASS_INFO);
	psDeviceNode->sDevId.eDeviceType = PVRSRV_DEVICE_TYPE_EXT;
	psDeviceNode->sDevId.eDeviceClass = PVRSRV_DEVICE_CLASS_DISPLAY;

	psDisplayClassInfo->ui32DeviceID = psDeviceNode->sDevId.ui32DeviceIndex;
	if (pui32DeviceID)
	{
		*pui32DeviceID = psDisplayClassInfo->ui32DeviceID;
	}
	return PVRSRV_OK;
}

IMG_EXPORT PVRSRV_ERROR mbxaccess_RegisterCmdProcList (IMG_UINT32 ui32DevIndex,
													PFN_CMD_PROC *ppfnCmdProcList,
													IMG_UINT32 ui32MaxSyncsPerCmd[][2],
													IMG_UINT32 ui32CmdCount)
{
	PVR_UNREFERENCED_PARAMETER(ui32DevIndex);
	PVR_UNREFERENCED_PARAMETER(ppfnCmdProcList);
	PVR_UNREFERENCED_PARAMETER(ui32MaxSyncsPerCmd);
	PVR_UNREFERENCED_PARAMETER(ui32CmdCount);
	return PVRSRV_OK;
}

IMG_EXPORT PVRSRV_ERROR mbxaccess_RemoveCmdProcList (IMG_UINT32 ui32DevIndex, IMG_UINT32 ui32CmdCount)
{
	PVR_UNREFERENCED_PARAMETER(ui32DevIndex);
	PVR_UNREFERENCED_PARAMETER(ui32CmdCount);
	return PVRSRV_OK;
}

IMG_EXPORT IMG_VOID mbxaccess_CommandComplete(IMG_HANDLE hCmdCookie)
{
	PVR_UNREFERENCED_PARAMETER(hCmdCookie);
	return;
}

PVRSRV_ERROR mbxaccess_GetLISRCommandCallback(PPFN_CMD_COMPLETE ppfnCmdCompleteCB)
{
	*ppfnCmdCompleteCB = VSyncCommandComplete;
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_RemoveDisplayClassDevice(IMG_UINT32 ui32DevIndex)
{
	PVRSRV_DEVICE_NODE       **ppsDevNode = IMG_NULL;
	PVRSRV_DEVICE_NODE        *psDevNode = IMG_NULL;
	PVRSRV_ERROR               eError = PVRSRV_OK;

	ppsDevNode = &gpsMBXDeviceList->psDeviceNodeList;
	if(IMG_NULL == ppsDevNode)
	{
		PVR_DPF((PVR_DBG_MESSAGE, "Device List is NULL"));
		eError = PVRSRV_ERROR_GENERIC;
	}
	else
	{
		while(*ppsDevNode)
		{
			switch((*ppsDevNode)->sDevId.eDeviceClass)
			{
				case PVRSRV_DEVICE_CLASS_DISPLAY :
				{
					PVRSRV_DISPLAYCLASS_INFO *psDisplayClassInfo = (*ppsDevNode)->pvDevice;

					if((*ppsDevNode)->sDevId.ui32DeviceIndex == ui32DevIndex)
					{
						psDevNode = *(ppsDevNode);
						psDisplayClassInfo = (PVRSRV_DISPLAYCLASS_INFO*)psDevNode->pvDevice;
						psDisplayClassInfo->pfnKSwapToSystem = IMG_NULL;
						psDisplayClassInfo->hSysSwapData = IMG_NULL;
						KFREE(psDisplayClassInfo);
						MBXDeviceNodeDestroy(psDevNode);
						return PVRSRV_OK;
					}
					break;
				}
				default:
				{
					break;
				}
			}
			ppsDevNode = &((*ppsDevNode)->psNext);
		}
		PVR_DPF((PVR_DBG_ERROR, "Requested device %d is not present", ui32DevIndex));
		eError = PVRSRV_ERROR_GENERIC;
	}

	return eError;
}

PVRSRV_SYNC_INFO *mbxaccess_CreateSyncObj(IMG_VOID)
{
	return (PVRSRV_SYNC_INFO*)IMG_NULL;
}

PVRSRV_ERROR mbxaccess_DestroySyncObj(PVRSRV_SYNC_INFO *psSyncInfo)
{
	PVR_UNREFERENCED_PARAMETER(psSyncInfo);
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_RegisterSystemISRHandler (PFN_ISR_HANDLER pfnISRHandler,
												IMG_VOID *pvISRHandlerData,
												IMG_UINT32 ui32ISRSourceMask,
												IMG_UINT32 ui32DeviceID)
{
	PVR_UNREFERENCED_PARAMETER(pfnISRHandler);
	PVR_UNREFERENCED_PARAMETER(pvISRHandlerData);
	PVR_UNREFERENCED_PARAMETER(ui32ISRSourceMask);
	PVR_UNREFERENCED_PARAMETER(ui32DeviceID);
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_SysOEMFunction(IMG_UINT32 ui32ID, IMG_VOID *pvIn, IMG_VOID *pvOut)
{
	PVR_UNREFERENCED_PARAMETER(ui32ID);
	PVR_UNREFERENCED_PARAMETER(pvIn);
	PVR_UNREFERENCED_PARAMETER(pvOut);
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_RegisterBufferClassDevice (IMG_CHAR*pszClientDrvName, IMG_UINT32*pui32DeviceID)
{
	PVRSRV_BUFFERCLASS_INFO *psBufferClassInfo = NULL;
	PVRSRV_DEVICE_NODE      *psDeviceNode = NULL;
	PVRSRV_ERROR             eError = PVRSRV_OK;
	
	psBufferClassInfo = KMALLOC(sizeof(PVRSRV_BUFFERCLASS_INFO), GFP_KERNEL);
	if(!(psBufferClassInfo))
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to alloc memory for psBufferClassInfo"));
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
	memset(psBufferClassInfo, 0, (size_t)sizeof(PVRSRV_BUFFERCLASS_INFO));

	
	strcpy(psBufferClassInfo->szClientDrvName, pszClientDrvName);
	psBufferClassInfo->DeviceClass = PVRSRV_DEVICE_CLASS_BUFFER;
	psBufferClassInfo->psBCInfoKM = psBufferClassInfo;

	
	eError = MBXDeviceNodeCreate(&psDeviceNode);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to alloc memory for DeviceNode"));
		KFREE(psBufferClassInfo);
		return eError;
	}

	psDeviceNode->pvDevice = (IMG_VOID*)psBufferClassInfo;
	psDeviceNode->ui32pvDeviceSize = sizeof(PVRSRV_BUFFERCLASS_INFO);
	psDeviceNode->sDevId.eDeviceType = PVRSRV_DEVICE_TYPE_EXT;
	psDeviceNode->sDevId.eDeviceClass = PVRSRV_DEVICE_CLASS_BUFFER;

	psBufferClassInfo->ui32DeviceID = psDeviceNode->sDevId.ui32DeviceIndex;
	if (pui32DeviceID)
	{
		*pui32DeviceID = psBufferClassInfo->ui32DeviceID;
	}
	return PVRSRV_OK;
}

PVRSRV_ERROR mbxaccess_RemoveBufferClassDevice(IMG_UINT32 ui32DevIndex)
{
	PVRSRV_DEVICE_NODE **ppsDevNode = IMG_NULL;
	PVRSRV_DEVICE_NODE  *psDevNode = IMG_NULL;
	PVRSRV_ERROR         eError = PVRSRV_OK;

	ppsDevNode = &gpsMBXDeviceList->psDeviceNodeList;
	if(IMG_NULL == ppsDevNode || IMG_NULL == *ppsDevNode)
	{
		PVR_DPF((PVR_DBG_MESSAGE, "Device List is NULL"));
		eError = PVRSRV_ERROR_GENERIC;
	}
	else
	{
		while(*ppsDevNode)
		{
			switch((*ppsDevNode)->sDevId.eDeviceClass)
			{
				case PVRSRV_DEVICE_CLASS_BUFFER :
				{
					PVRSRV_BUFFERCLASS_INFO *psBufferClassInfo = (*ppsDevNode)->pvDevice;
	
					if(psBufferClassInfo->ui32DeviceID == ui32DevIndex)
					{
						psDevNode = *(ppsDevNode);
						KFREE(psDevNode->pvDevice);
						MBXDeviceNodeDestroy(psDevNode);
						return PVRSRV_OK;
					}
					break;
				}
				default:
				{
					break;
				}
			}
			ppsDevNode = &((*ppsDevNode)->psNext);
		}
		PVR_DPF((PVR_DBG_ERROR, "Requested device %d is not present", ui32DevIndex));
		eError = PVRSRV_ERROR_GENERIC;
	}

	return eError;
}

IMG_EXPORT IMG_BOOL PVRGetDisplayClassJTable(PVRSRV_DC_KMJTABLE *psJTable)
{
	psJTable->pfnPVRSRVRegisterDisplayClassDevice = mbxaccess_RegisterDisplayClassDevice;
	psJTable->pfnPVRSRVRegisterCmdProcList = mbxaccess_RegisterCmdProcList;
	psJTable->pfnPVRSRVRemoveCmdProcList = mbxaccess_RemoveCmdProcList;
	psJTable->pfnPVRSRVGetLISRCommandCallback = mbxaccess_GetLISRCommandCallback;
	psJTable->pfnPVRSRVRemoveDisplayClassDevice = mbxaccess_RemoveDisplayClassDevice;
	psJTable->pfnPVRSRVCreateSyncObj = mbxaccess_CreateSyncObj;
	psJTable->pfnPVRSRVDestroySyncObj = mbxaccess_DestroySyncObj;
	psJTable->pfnPVRSRVRegisterSystemISRHandler = mbxaccess_RegisterSystemISRHandler;
	psJTable->pfnPVRSRVOEMFunction = mbxaccess_SysOEMFunction;

	psJTable->pfnPVRSRVCreateCommandQueue = DISPLAYCLASSJ_PVRSRVCREATECOMMANDQUEUEKM;
	psJTable->pfnPVRSRVDestroyCommandQueue = DISPLAYCLASSJ_PVRSRVDESTROYCOMMANDQUEUEKM;
	
	return IMG_TRUE;
}

IMG_EXPORT IMG_BOOL PVRGetBufferClassJTable(PVRSRV_BC_KMJTABLE *psJTable)
{
	psJTable->pfnPVRSRVRegisterBufferClassDevice = mbxaccess_RegisterBufferClassDevice;
	psJTable->pfnPVRSRVRegisterCmdProcList = mbxaccess_RegisterCmdProcList;
	psJTable->pfnPVRSRVRemoveCmdProcList = mbxaccess_RemoveCmdProcList;
	psJTable->pfnPVRSRVGetLISRCommandCallback = mbxaccess_GetLISRCommandCallback;
	psJTable->pfnPVRSRVRemoveBufferClassDevice = mbxaccess_RemoveBufferClassDevice;
	psJTable->pfnPVRSRVCreateSyncObj = mbxaccess_CreateSyncObj;
	psJTable->pfnPVRSRVDestroySyncObj = mbxaccess_DestroySyncObj;

	psJTable->pfnPVRSRVCreateCommandQueue = BUFFERCLASSJ_PVRSRVCREATECOMMANDQUEUEKM;
	psJTable->pfnPVRSRVDestroyCommandQueue = BUFFERCLASSJ_PVRSRVDESTROYCOMMANDQUEUEKM;

	return IMG_TRUE;
}

