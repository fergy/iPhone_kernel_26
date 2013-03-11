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

#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include "img_types.h"

typedef void *BM_HANDLE;

typedef struct _BM_STATE_ BM_STATE;

#define BP_POOL_MASK			0x7

#define BP_CONTIGUOUS			(1 << 3)
#define BP_PARAMBUFFER			(1 << 4)

#define BM_MAX_DEVMEM_ARENAS	2

IMG_BOOL
BM_Initialise(IMG_CPU_VIRTADDR registers, IMG_CPU_VIRTADDR slaveports,
			  IMG_UINT32 ui32CoreConfig);

IMG_VOID
BM_Finalise(IMG_VOID);

IMG_BOOL
BM_Reinitialise(IMG_VOID);

IMG_BOOL
BM_Alloc(IMG_SIZE_T uSize,
         IMG_UINT32 uFlags,
         IMG_UINT32 uDevVAddrAlignment,
         BM_HANDLE *phBuf);

IMG_BOOL
BM_Wrap(IMG_UINT32 ui32Size,
		IMG_UINT32 ui32Offset,
		IMG_UINT32 ui32PageCount,
		IMG_SYS_PHYADDR *psSysAddr,
		IMG_UINT32 uFlags,
		BM_HANDLE *phBuf);

void
BM_Free(BM_HANDLE hBuf);


IMG_CPU_VIRTADDR
BM_HandleToCpuVaddr(BM_HANDLE hBuf);

IMG_DEV_VIRTADDR
BM_HandleToDevVaddr(BM_HANDLE hBuf);

IMG_SYS_PHYADDR
BM_HandleToSysPaddr(BM_HANDLE hBuf);

IMG_BOOL
BM_ContiguousStatistics(IMG_UINT32 uFlags,
                        IMG_UINT32 *pTotalBytes,
                        IMG_UINT32 *pAvailableBytes);

#if defined(PDUMP) || defined(PDUMP2)
IMG_BOOL
BM_GetMMUPageTable(IMG_VOID **ppvMMUPageTableBaseKM);
#endif

#endif 

