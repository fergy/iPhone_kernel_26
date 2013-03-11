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

#ifndef MBXCCESSAPITYPE_H_ 
#define MBXCCESSAPITYPE_H_

typedef enum MBXTimerCBIDEnum
{
  MBXIDTIMERCB_MBXRECOVERHW,
  MBXIDTIMERCB_END
} MBXTimerCBID;

typedef enum MBXIOWrapperCBIDEnum
{
    MBXIOWrapperCBID_SYS = 0,
    MBXIOWrapperCBID_DIV = 1,
    MBXIOWrapperCBID_END
} MBXIOWrapperCBID;

typedef enum MBXProcWrapperCBIDEnum
{
    Dummy_CBID,
    MBXProcWrapperCBID_ResManPrintAllProcessResources,
    MBXProcWrapperCBID_MemHeapDescribeHeap,
    MBXProcWrapperCBID_MemHeapListHeap,
    
    MBXProcWrapperCBID_QueuePrintQueue,
    MBXProcWrapperCBID_ProcDumpVersion,
    MBXProcWrapperCBID_ProcDumpSysNodes,
    MBXProcWrapperCBID_PrintVirtualMemAllocations,
    MBXProcWrapperCBID_RA_DumpSegs,
    MBXProcWrapperCBID_RA_DumpInfo,
    MBXProcWrapperCBID_END
} MBXProcWrapperCBID;

typedef struct MBXDriverParamsTag
{
    unsigned int MBXDriverParam_PVRSRV_DEVICE_NODE_SIZE;
    unsigned int MBXDriverParam_SYS_DATA_SIZE;
    int debugLevel; 
} MBXDriverParams;

#endif 
