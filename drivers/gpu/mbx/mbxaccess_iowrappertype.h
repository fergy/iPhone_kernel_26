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

#ifndef MBXACCESS_IOWRAPPER_TYPE_H
#define MBXACCESS_IOWRAPPER_TYPE_H

typedef struct MBXUserDataForRequestIrqTag
{
	IMG_UINT32 irq;
	const char * devname;
	IMG_UINT16 size;
	IMG_UINT32 irqflags;
	void * dev_id;
	int ret;
} MBXUserDataForRequestIrq;

typedef struct MBXUserDataForFreeIrqTag
{
	IMG_UINT32 irq;
	void* data;
	IMG_UINT16 size;
} MBXUserDataForFreeIrq;

typedef struct MBXUserDataForHostMapPhysToLinTag
{
	IMG_CPU_PHYADDR offset;
	IMG_UINT32 size;
	IMG_UINT32 cacheType;
	IMG_CPU_VIRTADDR returnedIOMem;
} MBXUserDataForHostMapPhysToLin;

typedef struct MBXUserDataForIOUnmapTag
{
	IMG_UINT32 pvLinAddr;
} MBXUserDataForIOUnmap;

typedef struct MBXUserDataForPCIFindSlotTag
{
	IMG_UINT32 bus;
	IMG_UINT32 devfn;
	void* retPCIDevPointerInKS;
} MBXUserDataForPCIFindSlot;

typedef struct MBXUserDataForHostPCIReadWriteDwordTag
{
	IMG_UINT32 bus;
	IMG_UINT32 devfn;
	IMG_UINT32 func;
	IMG_UINT32 reg;
	IMG_UINT32 value;
} MBXUserDataForHostPCIReadWriteDword; 

typedef struct MBXUserDataForPageToPhysTag
{
    void* page; 
    IMG_UINT32 uiAddr; 
} MBXUserDataForPageToPhys;

#endif 
