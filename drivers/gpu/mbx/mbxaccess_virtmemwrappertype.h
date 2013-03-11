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

#ifndef MBXACCESS_VIRTMEMWRAPPER_TYPE_H
#define MBXACCESS_VIRTMEMWRAPPER_TYPE_H

typedef struct MBXUserDataForVirtualAllocateReserveTag
{
    IMG_UINT32 bytes; 
    IMG_BOOL cached; 
    void* mem;
} MBXUserDataForVirtualAllocateReserve; 
  
typedef struct MBXUserDataForVirtualDeallocateUnreserveTag
{
    void* mem; 
    int err;
} MBXUserDataForVirtualDeallocateUnreserve; 

typedef struct MBXUserDataForConvertKVToPageTag
{
    void* pvAddr;
    struct page* ret;
} MBXUserDataForConvertKVToPage;

#endif 
