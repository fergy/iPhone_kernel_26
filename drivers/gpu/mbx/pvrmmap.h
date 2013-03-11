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

#ifndef __PVRMMAP_H__
#define __PVRMMAP_H__


typedef struct PVRMEMMAPSTRUCT_TAG
{
    IMG_VOID	*pvUserAddress;
    IMG_VOID	*pvMappedAddress;
    IMG_UINT32	ui32Length;
    IMG_UINT32	ui32Offset;
    IMG_UINT32	ui32UsageCount;
    IMG_BOOL	bScattered;
} PVRMEMMAPSTRUCT;

PVRSRV_ERROR PVRMMAPInit(IMG_HANDLE hServices);

IMG_VOID *PVRMMAPMapKernelPtr(IMG_HANDLE hModule, IMG_VOID *pvKVAddress, IMG_UINT32 ui32Length);

IMG_VOID *PVRMMAPReverseMapping(IMG_VOID *pvUserAddress, IMG_UINT32 ui32Length);

IMG_BOOL PVRMMAPRemoveMapping(IMG_VOID *pvUserAddress, IMG_UINT32 ui32Length);

IMG_VOID PVRMMAPRemoveAllMappings(IMG_VOID);

PVRMEMMAPSTRUCT* PVRMMAPFindForwardMapping (IMG_VOID *pvKernelAddress, IMG_VOID *pvUserAddress, IMG_UINT32 ui32Length);


#endif 
