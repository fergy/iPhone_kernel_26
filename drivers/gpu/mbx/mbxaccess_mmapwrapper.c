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

#include <linux/poll.h>
#include <asm/io.h>

#include "services_headers.h"

#include "pvrmmap_private.h"
#include "mbxaccess_mmapwrappertype.h"
#include "mmap.h"
#include "mbxaccess_virtmemwrapper_private.h"

void MBXSyncSrv_MMapGetFullMapDataHandler(IMG_VOID *pvData)
{
	IMG_PVOID pvKVBase;
	IMG_UINT32 ui32BaseBytes;
	IMG_UINT32 ui32Offset;
	PVR_MMAP_TYPE eMapType;
	MBXUserDataForMMapGetFullMapData infoFromUser;
	
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForMMapGetFullMapData));

	infoFromUser.err = PVRMMapGetFullMapData(
								infoFromUser.pvKVAddr,
								infoFromUser.ui32Bytes,
								&pvKVBase,
								&ui32BaseBytes,
								&ui32Offset,
								&eMapType);

	infoFromUser.pvKVBase = pvKVBase;
	infoFromUser.ui32BaseBytes = ui32BaseBytes;
	infoFromUser.ui32Offset = ui32Offset;
	infoFromUser.eMapType = eMapType;

	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForMMapGetFullMapData));
}

void MBXSyncSrv_HostReadHWRegHandler(IMG_VOID *pvData)
{
	MBXUserDataForHostHWReg infoFromUser;
	
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostHWReg));

	infoFromUser.value = readl(infoFromUser.pBaseAddr+infoFromUser.offset);
	
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForHostHWReg));
}

void MBXSyncSrv_HostWriteHWRegHandler(IMG_VOID *pvData)
{
	MBXUserDataForHostHWReg infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostHWReg));

	writel(infoFromUser.value, infoFromUser.pBaseAddr + infoFromUser.offset);
	mb();
}

