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

#if !defined(__MMAP_H__)
#define __MMAP_H__

#include "mm.h"

#include "pvrmmap_private.h"


typedef struct KV_OFFSET_STRUCT_TAG
{
	PMEM_AREA					psMemArea;
	IMG_UINT32					ui32MemFlags;
	IMG_CPU_PHYADDR				CPUPhysAddr;
	IMG_UINT32					ui32MMapOffset;
	
	pid_t						pid;
	IMG_UINT16					ui16Mapped;
	const char					*pszName;
	
	
	struct KV_OFFSET_STRUCT_TAG	*psNext;
} KV_OFFSET_STRUCT, *PKV_OFFSET_STRUCT;

#define MAX_MMAP_AREAS 2048

PKV_OFFSET_STRUCT *PVRMMapRegisterArea(const char * name, PMEM_AREA psMemArea, IMG_UINT32 ui32Flags);

PVRSRV_ERROR PVRMMapRemoveRegisteredArea(IMG_VOID *pvMemArea);

PVRSRV_ERROR PVRMMapVerifyRegisteredAddress(IMG_VOID *pvMem);
PVRSRV_ERROR PVRMMapGetFullMapData(IMG_VOID *pvKVAddr,
										IMG_UINT32 ui32Bytes,
										IMG_PVOID *ppvKVBase,
										IMG_UINT32 *pui32BaseBytes,
										IMG_UINT32 *pui32Offset,
										PVR_MMAP_TYPE *peMapType);
struct file;
struct vm_area_struct;

int		PVRMMap(struct file* pFile, struct vm_area_struct* ps_vma);
void	PVRMMapInit(void);
void	PVRMMapCleanup(void);

#endif	
