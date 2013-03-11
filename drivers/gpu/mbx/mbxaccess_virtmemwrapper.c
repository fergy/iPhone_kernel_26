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
#endif

#include <linux/poll.h>

#include "services_headers.h"
#include "mbxaccess_virtmemwrapper_private.h"
#include "mbxaccess_virtmemwrappertype.h"

#include "pvr_debug.h"
#include "malloc_debug.h"
#include "mutex.h"
#include "mm.h"
#include "mbxaccess_virtmemwrapper.h"
#include "pvrmmap_private.h"
#include "mmap.h"

#if !defined(mem_map_reserve)
#define mem_map_reserve(p)	set_bit(PG_reserved, &((p)->flags))
#define mem_map_unreserve(p)	clear_bit(PG_reserved, &((p)->flags))
#endif


void MBXSyncSrv_VirtualAllocateReserveHandler(IMG_VOID *pvData)
{
	IMG_UINT32 ui32Flags = 0;
	PMEM_AREA psMemArea;

	MBXUserDataForVirtualAllocateReserve infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForVirtualAllocateReserve));

#if (defined(__arm__) && !defined(VIPT_MMAP_COLOUR))
	ui32Flags &= ~PVRSRV_HAP_CACHED;
#endif
	if(ui32Flags)
	{
		ui32Flags |= PVRSRV_HAP_USER_VISIBLE;
	}
	psMemArea = MEM_AREA_VMalloc(infoFromUser.bytes, ui32Flags);
	
	if(psMemArea == NULL)
	{
		return ;
	}

	
	if(!PVRMMapRegisterArea("*user-shared", psMemArea, ui32Flags & (PVRSRV_HAP_CACHED | PVRSRV_HAP_WRITECOMBINE)))
	{
		PVR_DPF((PVR_DBG_ERROR, "Unable to register area"));
		MEM_AREA_AreaDeepFree(psMemArea);
		infoFromUser.mem = NULL;
		return;
	}

	infoFromUser.mem = MEM_AREA_ToCpuVAddr(psMemArea);
	
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForVirtualAllocateReserve));
}

void MBXSyncSrv_VirtualDeallocateUnreserveHandler(IMG_VOID *pvData)
{
	MBXUserDataForVirtualDeallocateUnreserve infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForVirtualDeallocateUnreserve));

	PVRMMapRemoveRegisteredArea(infoFromUser.mem);
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForVirtualDeallocateUnreserve));
}

void MBXSyncSrv_ConvertKVToPageHandler(IMG_VOID *pvData)
{
	MBXUserDataForConvertKVToPage infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForConvertKVToPage));
	infoFromUser.ret = ConvertKVToPage(infoFromUser.pvAddr);
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForConvertKVToPage));
}


void ReservePages(IMG_VOID *pvAddress, IMG_UINT32 ui32Length)
{
	IMG_VOID *pvPage;
	IMG_VOID *pvEnd = pvAddress + ui32Length;

	for(pvPage = pvAddress; pvPage < pvEnd;  pvPage += PAGE_SIZE)
	{
 #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
		SetPageReserved(ConvertKVToPage(pvPage));
 #else
		mem_map_reserve(ConvertKVToPage(pvPage));
 #endif
	}
}

void UnreservePages(IMG_VOID *pvAddress, IMG_UINT32 ui32Length)
{
	IMG_VOID *pvPage;
	IMG_VOID *pvEnd = pvAddress + ui32Length;

	for(pvPage = pvAddress; pvPage < pvEnd;  pvPage += PAGE_SIZE)
	{
 #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
		ClearPageReserved(ConvertKVToPage(pvPage));
 #else
		mem_map_unreserve(ConvertKVToPage(pvPage));
 #endif
	}
}

