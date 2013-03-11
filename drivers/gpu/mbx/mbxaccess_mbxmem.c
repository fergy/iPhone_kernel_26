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

#include <linux/sched.h>
#include <linux/poll.h>

#include "services.h"
#include "mbxaccess_mbxsysdata.h"
#include "pvr_debug.h"
#include "mbxaccess_mbxmem.h"
#include "mbxaccess_devicewrappertype.h"
#include "mbxworker_thread_struct.h"
#include "mbxdisplay.h"
#include "mbxaccess_virtmemwrapper_private.h"

#include "malloc_debug.h"

MBX_DEVICELIST *gpsMBXDeviceList = IMG_NULL;

extern MBX_WORKER_THREAD_STRUCT *gpsInterruptStructKernel;

static PVRSRV_ERROR MBXAllocateDeviceID(IMG_UINT32 *pui32DevID);
static PVRSRV_ERROR MBXFreeDeviceID(IMG_UINT32 ui32DevID);
static PVRSRV_ERROR MBXGetDeviceNodeValue(IMG_UINT32 ui32ID, MBXDeviceNodeField eField, IMG_VOID** ppValue);
static PVRSRV_ERROR MBXSetDeviceNodeValue( IMG_UINT32 ui32ID, MBXDeviceNodeField eField, IMG_VOID* pValue);

static IMG_VOID MBXGetDeviceNodeId(PVRSRV_DEVICE_NODE **ppsDeviceNode, IMG_UINT32 ui32DevId)
{
	if(IMG_NULL != gpsMBXDeviceList)
	{
		*ppsDeviceNode = gpsMBXDeviceList->psDeviceNodeList;
		while(IMG_NULL != *ppsDeviceNode && ui32DevId != (*ppsDeviceNode)->sDevId.ui32DeviceIndex)
		{
			*ppsDeviceNode = (*ppsDeviceNode)->psNext;
		}
	}
}

static IMG_UINT32 *MBXGetDeviceIndexList(PVRSRV_DEVICE_CLASS eDeviceClass, IMG_UINT32 *pui32DevCount)
{
	IMG_UINT32         *ui32DevIdList = IMG_NULL;
	PVRSRV_DEVICE_NODE *psDeviceNode = IMG_NULL;
	IMG_UINT32          iCounter = 0x0000;

	ui32DevIdList = (IMG_UINT32*)KMALLOC(sizeof(IMG_UINT32) * SYS_DEVICE_COUNT, GFP_USER);
	memset(ui32DevIdList, 0x0000, (size_t)(sizeof(IMG_UINT32) * SYS_DEVICE_COUNT));

	psDeviceNode = gpsMBXDeviceList->psDeviceNodeList;
	while(psDeviceNode)
	{
		if(eDeviceClass == psDeviceNode->sDevId.eDeviceClass)
		{
			ui32DevIdList[iCounter] = psDeviceNode->sDevId.ui32DeviceIndex;
			iCounter++;
		}
		psDeviceNode = psDeviceNode->psNext;
	}
	
	*pui32DevCount = iCounter;
	
	if(!iCounter)
	{
		PVR_DPF((PVR_DBG_WARNING, "Device List is empty"));
        KFREE(ui32DevIdList);
		ui32DevIdList = IMG_NULL;
	}

	if(!ui32DevIdList)
	{
		PVR_DPF((PVR_DBG_WARNING, "Device List is empty"));
	}

	return ui32DevIdList;
}

static PVRSRV_ERROR MBXAllocateDeviceID(IMG_UINT32 *pui32DevID)
{
	SYS_DEVICE_ID *psDeviceWalker = NULL;
	SYS_DEVICE_ID *psDeviceEnd = NULL;

	psDeviceWalker = &gpsMBXDeviceList->sDeviceID[0];
	psDeviceEnd = psDeviceWalker + gpsMBXDeviceList->ui32NumDevices;

	
	while (psDeviceWalker < psDeviceEnd)
	{
		if (!psDeviceWalker->bInUse)
		{
			psDeviceWalker->bInUse = IMG_TRUE;
			*pui32DevID = psDeviceWalker->uiID;
			return PVRSRV_OK;
		}
		psDeviceWalker++;
	}

	PVR_DPF((PVR_DBG_ERROR," %s: No free and valid device IDs available!",__FUNCTION__));

	


	PVR_ASSERT(0);
	return PVRSRV_ERROR_GENERIC;
}

static PVRSRV_ERROR MBXFreeDeviceID(IMG_UINT32 ui32DevID)
{
	SYS_DEVICE_ID *psDeviceWalker = NULL;
	SYS_DEVICE_ID *psDeviceEnd = NULL;

	psDeviceWalker = &gpsMBXDeviceList->sDeviceID[0];
	psDeviceEnd = psDeviceWalker + gpsMBXDeviceList->ui32NumDevices;

	
	while (psDeviceWalker < psDeviceEnd)
	{
		if((psDeviceWalker->uiID == ui32DevID) && (psDeviceWalker->bInUse))
		{
			psDeviceWalker->bInUse = IMG_FALSE;
			return PVRSRV_OK;
		}
		psDeviceWalker++;
	}

	PVR_DPF((PVR_DBG_ERROR," %s: No matching dev ID that is in use!",__FUNCTION__));

	
	PVR_ASSERT(0);
	return PVRSRV_ERROR_GENERIC;
}

PVRSRV_ERROR MBXDeviceNodeCreate (PVRSRV_DEVICE_NODE **ppsDeviceNode)
{
	PVRSRV_ERROR        eError = PVRSRV_OK;
	IMG_UINT32          ui32DeviceID = 0xFFFF;

	eError = MBXAllocateDeviceID(&ui32DeviceID);
	if (eError != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR," %s: Failed to allocate ID",__FUNCTION__));
		return eError;
	}

	
    *ppsDeviceNode = (PVRSRV_DEVICE_NODE*)KMALLOC(sizeof(PVRSRV_DEVICE_NODE), GFP_KERNEL);
	if(!(*ppsDeviceNode))
	{
		PVR_DPF((PVR_DBG_ERROR," %s: Failed to alloc memory for DeviceNode",__FUNCTION__));
		MBXFreeDeviceID(ui32DeviceID);
		return (PVRSRV_ERROR_OUT_OF_MEMORY);
	}

	memset(*ppsDeviceNode, 0, (size_t)sizeof(PVRSRV_DEVICE_NODE));

	(*ppsDeviceNode)->ui32RefCount = 1;
	(*ppsDeviceNode)->sDevId.ui32DeviceIndex = ui32DeviceID;

	
	(*ppsDeviceNode)->psNext = gpsMBXDeviceList->psDeviceNodeList;
	gpsMBXDeviceList->psDeviceNodeList = *ppsDeviceNode;

	return PVRSRV_OK;
}

void MBXDeviceNodeDestroy(PVRSRV_DEVICE_NODE *psDeviceNode)
{
	PVRSRV_DEVICE_NODE **ppsDevNode = NULL;

	
	ppsDevNode = &gpsMBXDeviceList->psDeviceNodeList;
	if (*ppsDevNode == psDeviceNode)
	{
		*ppsDevNode = psDeviceNode->psNext;
	}
	else
	{
		while (*ppsDevNode)
		{
			if((*ppsDevNode)->psNext == psDeviceNode)
			{
				(*ppsDevNode)->psNext = psDeviceNode->psNext;
				break;
			}
			*ppsDevNode = (*ppsDevNode)->psNext;
		}
	}

	
	MBXFreeDeviceID(psDeviceNode->sDevId.ui32DeviceIndex);

	
    KFREE(psDeviceNode);
}

void MBXSyncSrv_DeviceNodeCreate(IMG_VOID *pvData)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
	MBXUserDataForDeviceNodeCreate infoFromUser;
	PVRSRV_ERROR status;
	
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForDeviceNodeCreate));

	psDeviceNode = IMG_NULL;
	status = MBXDeviceNodeCreate(&psDeviceNode);
	if(psDeviceNode)
	{
		infoFromUser.ui32DevId = psDeviceNode->sDevId.ui32DeviceIndex;
		infoFromUser.ret = status;
	}
	else
	{
		infoFromUser.ui32DevId = 0xFFFF;
		infoFromUser.ret = PVRSRV_ERROR_GENERIC;
	}

	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForDeviceNodeCreate));
}


void MBXSyncSrv_DeviceNodeDestroy(IMG_VOID *pvData)
{
	PVRSRV_DEVICE_NODE *psDeviceNode;
	MBXUserDataForDeviceNodeDestroy infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForDeviceNodeDestroy));
	MBXGetDeviceNodeId(&psDeviceNode, infoFromUser.ui32DevId);

	MBXDeviceNodeDestroy(psDeviceNode);
}


void MBXSyncSrv_GetDeviceNodeValue(IMG_VOID *pvData)
{
	PVRSRV_ERROR status;
	
	MBXUserDataForDeviceFieldGet infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForDeviceFieldGet));
	status = MBXGetDeviceNodeValue(infoFromUser.ui32ID,
												infoFromUser.eField,
												infoFromUser.ppValue);
	infoFromUser.ret = status;
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForDeviceFieldGet));
}


void MBXSyncSrv_SetDeviceNodeValue(IMG_VOID *pvData)
{
	PVRSRV_ERROR status;
	
	MBXUserDataForDeviceFieldSet infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForDeviceFieldSet));
	status = MBXSetDeviceNodeValue(infoFromUser.ui32ID,
												infoFromUser.eField,
												infoFromUser.pValue);
	infoFromUser.ret = status;
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForDeviceFieldSet));
}

static PVRSRV_ERROR MBXGetDeviceNodeValue(IMG_UINT32 ui32ID, MBXDeviceNodeField eField, IMG_VOID** ppValue)
{
	PVRSRV_ERROR        ret = PVRSRV_OK;
	PVRSRV_DEVICE_NODE *psDeviceNode;
	
	if(!ppValue)
	{
		PVR_DPF((PVR_DBG_ERROR," %s: PASSED VALUE IS NULL",__FUNCTION__));
		return PVRSRV_ERROR_GENERIC;
	}

	MBXGetDeviceNodeId(&psDeviceNode, ui32ID);

	if(IMG_NULL == psDeviceNode)
	{
		PVR_DPF((PVR_DBG_ERROR," %s: Device Node not found",__FUNCTION__));
		ret = PVRSRV_ERROR_GENERIC;
	}
	else
	{
		switch(eField)
		{
			case MBXDEVICENODEFIELD_DEVICE_NODE:
				*ppValue = (IMG_VOID*)psDeviceNode;
				break;
			case MBXDEVICENODEFIELD_DEVICE_TYPE:
				*ppValue = (IMG_VOID*)psDeviceNode->sDevId.eDeviceType;
				break;
			case MBXDEVICENODEFIELD_DEVICE_CLASS:
				*ppValue = (IMG_VOID*)psDeviceNode->sDevId.eDeviceClass;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INDEX:
				*ppValue = (IMG_VOID*)psDeviceNode->sDevId.ui32DeviceIndex;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INDEX_LIST_COUNT:
			{
				IMG_UINT32 ui32DevCount = 0x0000;
				IMG_UINT32 *psDeviceList = MBXGetDeviceIndexList((PVRSRV_DEVICE_CLASS)(*ppValue), &ui32DevCount);
                KFREE(psDeviceList);
				*ppValue = (IMG_VOID*)ui32DevCount;
				break;
			}
			case MBXDEVICENODEFIELD_DEVICE_INDEX_LIST:
			{
				IMG_UINT32 ui32DevCount = 0x0000;
				PVRSRV_DEVICE_CLASS *pTemp = (PVRSRV_DEVICE_CLASS*)(*ppValue);
				IMG_UINT32 *psDeviceList = MBXGetDeviceIndexList(*pTemp, &ui32DevCount);
				COPY_TO_USER(*ppValue, (IMG_VOID*)psDeviceList, sizeof(IMG_UINT32) * ui32DevCount);
                KFREE(psDeviceList);
				break;
			}
			case MBXDEVICENODEFIELD_REF_COUNT:
				*ppValue = (IMG_VOID*)psDeviceNode->ui32RefCount;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_INIT:
				*ppValue = (IMG_VOID*)psDeviceNode->pfnInitDevice;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_DEINIT:
				*ppValue = (IMG_VOID*)psDeviceNode->pfnDeInitDevice;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_PHYS_BASE:
				*ppValue = (IMG_VOID*)psDeviceNode->pfnGetDevSysPhysBase;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_ISR:
				*ppValue = (IMG_VOID*)psDeviceNode->pfnDeviceISR;
				break;
			case MBXDEVICENODEFIELD_DEVICE_ISR_DATA:
				*ppValue = (IMG_VOID*)psDeviceNode->pvISRData;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INFO:
				*ppValue = (IMG_VOID*)psDeviceNode->pvDevice;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INFO_US:
			{
				COPY_TO_USER(*ppValue, psDeviceNode->pvDevice, psDeviceNode->ui32pvDeviceSize);
				break;
			}
			case MBXDEVICENODEFIELD_DEVICE_SIZE:
				*ppValue = (IMG_VOID*)psDeviceNode->ui32pvDeviceSize;
				break;
			case MBXDEVICENODEFIELD_DEVICE_RES_ITEM:
				*ppValue = (IMG_VOID*)psDeviceNode->psResItem;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INTERRUPT_STRUCT:
				*ppValue = (IMG_VOID*)psDeviceNode->pvInterruptStruct;
				break;
			case MBXDEVICENODEFIELD_DEVICE_RESMAN_INFO:
				((PVRSRV_DC_RESMAN_INFO*)(*ppValue))->pfnKSwapToSystem = ((PVRSRV_DISPLAYCLASS_INFO*)psDeviceNode->pvDevice)->pfnKSwapToSystem;
				((PVRSRV_DC_RESMAN_INFO*)(*ppValue))->hSysSwapData = ((PVRSRV_DISPLAYCLASS_INFO*)psDeviceNode->pvDevice)->hSysSwapData;
				break;
			default:
				*ppValue = IMG_NULL;
				ret = PVRSRV_ERROR_GENERIC;
				break;
		}
	}
	return ret;
}

static PVRSRV_ERROR MBXSetDeviceNodeValue(IMG_UINT32 ui32ID, MBXDeviceNodeField eField, IMG_VOID* pValue)
{
	PVRSRV_ERROR        ret = PVRSRV_OK;
	PVRSRV_DEVICE_NODE *psDeviceNode;

	MBXGetDeviceNodeId(&psDeviceNode, ui32ID);

	if(IMG_NULL == psDeviceNode)
	{
		PVR_DPF((PVR_DBG_ERROR,"%s: Device Node not found",__FUNCTION__));
		ret = PVRSRV_ERROR_GENERIC;
	}
	else
	{
		switch(eField)
		{
			case MBXDEVICENODEFIELD_DEVICE_TYPE:
				psDeviceNode->sDevId.eDeviceType = (PVRSRV_DEVICE_TYPE)pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_CLASS:
				psDeviceNode->sDevId.eDeviceClass = (PVRSRV_DEVICE_CLASS)pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INDEX:
				psDeviceNode->sDevId.ui32DeviceIndex = (IMG_UINT32)pValue;
				break;
			case MBXDEVICENODEFIELD_REF_COUNT:
				psDeviceNode->ui32RefCount = (IMG_UINT32)pValue;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_INIT:
				psDeviceNode->pfnInitDevice = pValue;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_DEINIT:
				psDeviceNode->pfnDeInitDevice = pValue;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_PHYS_BASE:
				psDeviceNode->pfnGetDevSysPhysBase = pValue;
				break;
			case MBXDEVICENODEFIELD_PFN_DEVICE_ISR:
				psDeviceNode->pfnDeviceISR = pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_ISR_DATA:
				psDeviceNode->pvISRData = pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INFO:
				psDeviceNode->pvDevice = pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_SIZE:
				psDeviceNode->ui32pvDeviceSize = (IMG_UINT32)pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_RES_ITEM:
				psDeviceNode->psResItem = (PRESMAN_ITEM)pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INFO_DISPLAY:
				((PVRSRV_DISPLAYCLASS_INFO*)psDeviceNode->pvDevice)->hSysSwapData = pValue;
				break;
			case MBXDEVICENODEFIELD_DEVICE_INTERRUPT_STRUCT:
				psDeviceNode->pvInterruptStruct = pValue;
				gpsInterruptStructKernel = (MBX_WORKER_THREAD_STRUCT*)(psDeviceNode->pvInterruptStruct);
				break;
			case MBXDEVICENODEFIELD_DEVICE_PVRSRV_INFO_DISPLAY:
				((PVRSRV_DISPLAYCLASS_INFO*)psDeviceNode->pvDevice)->pfnKSwapToSystem = ((PVRSRV_DC_PVRSRV_INFO*)pValue)->pfnSwapToSystem;
				strcpy(((PVRSRV_DISPLAYCLASS_INFO*)psDeviceNode->pvDevice)->szClientDrvName, ((PVRSRV_DC_PVRSRV_INFO*)pValue)->szClientDrvName);
				break;
			case MBXDEVICENODEFIELD_DEVICE_PVRSRV_DEVICE_DELETE:
                KFREE(psDeviceNode->pvDevice);
				psDeviceNode->pvDevice = IMG_NULL;
				break;
			default:
				ret = PVRSRV_ERROR_GENERIC;
				break;
		}
	}
	return ret;
}

