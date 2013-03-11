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

#ifndef __DEVICE_H__
#define __DEVICE_H__

#if defined(UBUILD)

typedef struct _PVRSRV_DEVICE_NODE_
{
	PVRSRV_DEVICE_IDENTIFIER	sDevId;
	IMG_UINT32					ui32RefCount;

	
	PVRSRV_ERROR            (*pfnInitDevice) (IMG_VOID*, IMG_UINT32 ui32DevId);    
	PVRSRV_ERROR				(*pfnDeInitDevice) (IMG_VOID*);	
	IMG_SYS_PHYADDR*			(*pfnGetDevSysPhysBase)(IMG_VOID*);
	IMG_VOID                (*pfnDeviceISR)(IMG_VOID*, IMG_UINT32 ui32MBXIntEnable);
	IMG_VOID					*pvISRData;

	
	IMG_VOID					*pvDevice;
	IMG_UINT32					ui32pvDeviceSize; 

	
	PRESMAN_ITEM				psResItem;
	IMG_VOID                *pvInterruptStruct;
	struct _PVRSRV_DEVICE_NODE_	*psNext;
} PVRSRV_DEVICE_NODE;
#endif

typedef struct _SYS_DATA_TAG_ *PSYS_DATA;

PVRSRV_ERROR PVRSRVRegisterDevice (PSYS_DATA psSysData,
								   PVRSRV_ERROR (*pfnRegisterDevice)(IMG_UINT32),
								   IMG_UINT32 *pui32DeviceIndex );

PVRSRV_ERROR PVRSRVInitialiseDevice(IMG_UINT32 ui32DevIndex);

PVRSRV_ERROR PVRSRVDeinitialiseDevice(IMG_UINT32 ui32DevIndex);

PVRSRV_ERROR PollForValueKM (volatile IMG_UINT32* pui32LinMemAddr,
									  IMG_UINT32 ui32Value,
									  IMG_UINT32 ui32Mask,
									  IMG_UINT32 ui32Waitus,
									  IMG_UINT32 ui32Tries);

#endif 

