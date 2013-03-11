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

#ifndef _MBXACCESS_MBXMEM_H_
#define _MBXACCESS_MBXMEM_H_

#include <linux/mm.h>

typedef struct _MBX_DEVICELIST_
{
    PVRSRV_DEVICE_NODE *psDeviceNodeList;
    SYS_DEVICE_ID       sDeviceID[SYS_DEVICE_COUNT];
    IMG_UINT32          ui32NumDevices;
} MBX_DEVICELIST;

extern MBX_DEVICELIST *gpsMBXDeviceList;

void MBXSyncSrv_DeviceNodeCreate(IMG_VOID *pvData);
void MBXSyncSrv_DeviceNodeDestroy(IMG_VOID *pvData);
void MBXSyncSrv_GetDeviceNodeValue(IMG_VOID *pvData);
void MBXSyncSrv_SetDeviceNodeValue(IMG_VOID *pvData);

PVRSRV_ERROR MBXDeviceNodeCreate (PVRSRV_DEVICE_NODE **ppsDeviceNode);
void MBXDeviceNodeDestroy(PVRSRV_DEVICE_NODE *psDeviceNode);

#endif 
