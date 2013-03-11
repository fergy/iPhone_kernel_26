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
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>

#include "pvrsrverror.h"
#include "pvr_debug.h"
#include "mbxaccess_memwrappertype.h"
#include "mbxaccess_virtmemwrapper_private.h"

static PVRSRV_ERROR HostFlushCpuCacheArea(struct vm_area_struct * psVma,
				IMG_UINT32 ui32Start, IMG_UINT32 ui32End, IMG_UINT32 ui32Flags);

static PVRSRV_ERROR PVRSRVFlushCpuCacheKM(IMG_UINT32 ui32Address,
				IMG_UINT32 ui32LengthBytes,
				IMG_UINT32 ui32Flags);

void MBXSyncSrv_HostFlushCpuCacheAreaHandler(IMG_VOID *pvData)
{
	MBXUserDataForHostFlushCpuCacheArea infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForHostFlushCpuCacheArea));
	infoFromUser.err = HostFlushCpuCacheArea(infoFromUser.vma,infoFromUser.start,infoFromUser.end,infoFromUser.flags);

	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForHostFlushCpuCacheArea));
}

void MBXSyncSrv_PVRSRVFlushCpuCacheKMHandler(IMG_VOID *pvData)
{
	MBXUserDataForPVRSRVFlushCpuCacheKM infoFromUser;

	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForPVRSRVFlushCpuCacheKM));

	infoFromUser.err = PVRSRVFlushCpuCacheKM(infoFromUser.address, infoFromUser.lengthBytes, infoFromUser.flags);
	
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForPVRSRVFlushCpuCacheKM));
}

IMG_UINT32 HostGetPageSize(IMG_VOID)
{
	return PAGE_SIZE;
}

PVRSRV_ERROR PVRSRVFlushCpuCacheKM(IMG_UINT32 ui32Address,
				IMG_UINT32 ui32LengthBytes,
				IMG_UINT32 ui32Flags)
{
	struct vm_area_struct * psVma;
	IMG_UINT32 ui32Start = ui32Address & PAGE_MASK;
	IMG_UINT32 ui32Len = (ui32LengthBytes + ~PAGE_MASK) & PAGE_MASK;
	IMG_UINT32 ui32End = ui32Start + ui32Len;

	psVma = find_vma(
	  current->mm, 
	  (u32) ui32Start);

	for (;;) 
	{	
		if (!psVma)
		{
			break;
		}
		
		if (ui32Start < psVma->vm_start) 
		{
			ui32Start = psVma->vm_start;
		}
		
		if (ui32End <= psVma->vm_end) 
		{
			if (ui32Start < ui32End) 
			{
				HostFlushCpuCacheArea(psVma, ui32Start, ui32End, ui32Flags);
			}
			break;
		}

		
		HostFlushCpuCacheArea(psVma, ui32Start, psVma->vm_end, ui32Flags);
		ui32Start = psVma->vm_end;
		psVma = psVma->vm_next;
	}
	return PVRSRV_OK;
}

PVRSRV_ERROR HostFlushCpuCacheArea(struct vm_area_struct * psVma,
				IMG_UINT32 ui32Start, IMG_UINT32 ui32End, IMG_UINT32 ui32Flags)
{
	IMG_UINT32 ui32Address;
	spin_lock(&psVma->vm_mm->page_table_lock);

	if (current->active_mm == psVma->vm_mm)
	{
		ui32Start &= PAGE_MASK;
		ui32End = PAGE_ALIGN(ui32End);
		for (ui32Address = ui32Start; ui32Address < ui32End; ui32Address += PAGE_SIZE)
		{
#if defined(__arm__)
			__asm__ ("mov   r0, %0\n\t"
				  "mov    r1, %1\n\t"
				  "sub    r1, r1, #1\n\t"
				  "mcrr   p15, 0, r1, r0, c5  @ invalidate I-cache range      \n\t"
				  "mov    r1, #0                              \n\t"
				  "mcr    p15, 0, r1, c7, c10, 4  @ drain the write buffer        \n\t"
				  :
				  : "r" (ui32Address), "r" (ui32Address + PAGE_SIZE)
				  : "r0", "r1");
#else
#error "mbxaccess_memwrapper.c - HostFlushCpuCacheArea - Unknown architecture."
#endif
		}
	}
	spin_unlock(&psVma->vm_mm->page_table_lock);
	return PVRSRV_OK;
}
