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

#ifndef MBXACCESS_VIRTMEMWRAPPER_PRIVATE_H
#define MBXACCESS_VIRTMEMWRAPPER_PRIVATE_H

#define COPY_TO_USER(arg, addr, size) \
	if(copy_to_user(arg, addr, size) != 0) { \
		PVR_DPF((PVR_DBG_ERROR, "Copy to user failed.")); \
	}

#define COPY_FROM_USER(arg, addr, size) \
	if(copy_from_user(arg, addr, size) != 0) { \
		PVR_DPF((PVR_DBG_ERROR, "Copy from user failed.")); \
	}


typedef struct tag_VirtAllocRec
{
	struct tag_VirtAllocRec	*psNext;
	IMG_VOID				*pvMem;
	IMG_UINT32				ui32Bytes;
	pid_t					pid;
} VIRT_ALLOC_REC, *PVIRT_ALLOC_REC;

IMG_VOID * VirtualAllocateReserve(IMG_UINT32 ui32Bytes, IMG_BOOL bCached);
PVRSRV_ERROR VirtualDeallocateUnreserve(IMG_VOID *pvMem);

void ReservePages(IMG_VOID *pvAddress, IMG_UINT32 ui32Length);
void UnreservePages(IMG_VOID *pvAddress, IMG_UINT32 ui32Length);
struct page *ConvertKVToPage(IMG_PVOID pvAddr);
IMG_VOID VirtualMemoryCleanup(IMG_VOID);

#endif 
