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

#ifndef __MBXACCESS_INTERNALS_H__
#define __MBXACCESS_INTERNALS_H__

#ifdef __X86_BUILD__
#define __USER__ __user
#else
#define __USER__
#endif

struct file;
struct inode;

typedef enum MBXKernelSyncServiceIDEnum
{
	
	 MBXKERNELSYNCSERVICE_REQUESTIRQ =0 ,
	 MBXKERNELSYNCSERVICE_FREEIRQ,
	 MBXKERNELSYNCSERVICE_HOSTMAPPHYSTOLIN,
	 MBXKERNELSYNCSERVICE_PCIFINDSLOT,
	 MBXKERNELSYNCSERVICE_HOSTPCIREADDWORD,

	
	 MBXKERNELSYNCSERVICE_PAGETOPHYS,

	
	 MBXKERNELSYNCSERVICE_VIRTUALALLOCATERESERVE,
	 MBXKERNELSYNCSERVICE_VIRTUALDEALLOCATEUNRESERVE,
	 MBXKERNELSYNCSERVICE_CONVERTKVTOPAGE,

	
	 MBXKERNELSYNCSERVICE_HOSTFLUSHCPUCACHEAREA,
	 MBXKERNELSYNCSERVICE_PVRSRVFLUSHCPUCACHEKM,

	
	 MBXKERNELSYNCSERVICE_CREATEPROCENTRIES,
	 MBXKERNELSYNCSERVICE_REMOVEPROCENTRIES,

	
	 MBXKERNELSYNCSERVICE_HOSTADDTIMER,
	 MBXKERNELSYNCSERVICE_HOSTREMOVETIMER,

	
	 MBXKERNELSYNCSERVICE_MMAPGETFULLMAPDATA,
	 MBXKERNELSYNCSERVICE_HOSTREADHWREG,
	 MBXKERNELSYNCSERVICE_HOSTWRITEHWREG,

	
	 MBXKERNELSYNCSERVICE_LOADDRIVERPARAMS,

	
	 MBXKERNELSYNCSERVICE_DEVICENODECREATE,
	 MBXKERNELSYNCSERVICE_DEVICENODEDESTROY,
	 MBXKERNELSYNCSERVICE_GETDEVICENODEVALUE,
	 MBXKERNELSYNCSERVICE_SETDEVICENODEVALUE,

	 MBXKERNELSYNCSERVICE_SERVICEAPIGETINITSTATUS,
	 MBXKERNELSYNCSERVICE_SERVICEAPISETINITSTATUS,
#if defined(DEBUG)
	 MBXKERNELSYNCSERVICE_LINEIDENTIFIER,
#endif
	MBXKERNELSYNCSERVICE_END
} MBXKernelSyncServiceID;

typedef struct MBXInfoForSyncServiceTag
{
	MBXKernelSyncServiceID id; 
	void* data; 
} MBXInfoForKernelSyncService;
#endif 

