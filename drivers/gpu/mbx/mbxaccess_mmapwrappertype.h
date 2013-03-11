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

#ifndef MBXACCESS_MMAPWRAPPER_TYPE_H
#define MBXACCESS_MMAPWRAPPER_TYPE_H

typedef struct MBXUserDataForMMapGetFullMapDataTag
{
    
    IMG_VOID *pvKVAddr;
    IMG_UINT32 ui32Bytes;
    
    IMG_PVOID *pvKVBase;
    IMG_UINT32 ui32BaseBytes;
    IMG_UINT32 ui32Offset;
    PVR_MMAP_TYPE eMapType;
    
    PVRSRV_ERROR err;
} MBXUserDataForMMapGetFullMapData; 

typedef struct MBXUserDataForHostHWRegTag
{
    IMG_PVOID pBaseAddr;
    IMG_UINT32 offset;
    IMG_UINT32 value;
} MBXUserDataForHostHWReg;

typedef struct MBXUserDataForRegisterAreaTag
{
    const char     *name;
    IMG_UINT32	   stringsize;
    IMG_VOID       *pvArea;
    IMG_CPU_PHYADDR CPUPhysAddr;
    IMG_UINT32      ui32Bytes;
    PVR_MMAP_TYPE   eMapType;
    IMG_BOOL        bCached;
    IMG_BOOL        bWriteCombined;
} MBXUserDataForRegisterArea;

typedef struct MBXUserDataForRemoveAreaTag
{
    IMG_VOID    *pvVirtAddress;
    PVRSRV_ERROR err;
} MBXUserDataForRemoveArea;

#endif 
