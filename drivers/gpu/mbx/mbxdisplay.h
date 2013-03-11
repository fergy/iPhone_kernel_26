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

#ifndef _MBXDISPLAY_H_
#define _MBXDISPLAY_H_

typedef struct PVRSRV_DC_PVRSRV_INFO_TAG
{
    PFN_SWAP_TO_SYSTEM  pfnSwapToSystem;
    IMG_CHAR           *szClientDrvName;
} PVRSRV_DC_PVRSRV_INFO;

typedef struct PVRSRV_DC_RESMAN_INFO_TAG
{
    PFN_SWAP_TO_SYSTEM pfnKSwapToSystem;
    IMG_HANDLE         hSysSwapData;
} PVRSRV_DC_RESMAN_INFO;

typedef PVRSRV_ERROR (*PFN_DC_MBX_CREATE_QUEUE) (IMG_UINT32, PVRSRV_QUEUE_INFO **);

typedef PVRSRV_ERROR      (*PFN_DC_MBX_REGISTER_COMMANDPROCLIST)(IMG_UINT32, PPFN_CMD_PROC,IMG_UINT32[][2], IMG_UINT32);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_REMOVE_COMMANDPROCLIST)(IMG_UINT32, IMG_UINT32);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_GET_LISR_CMDCB)(PPFN_CMD_COMPLETE);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_DESTROY_QUEUE)(PVRSRV_QUEUE_INFO*);
typedef PVRSRV_SYNC_INFO* (*PFN_DC_MBX_CREATE_SYNCOBJ)(IMG_VOID);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_DESTROY_SYNCOBJ)(PVRSRV_SYNC_INFO*);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_SET_DEVICE_INFO)(IMG_VOID*, IMG_UINT32);
typedef PVRSRV_ERROR      (*PFN_DC_MBX_SET_PVRSRV_INFO)(IMG_BOOL (*pfnKSwapToSystem)(IMG_HANDLE), IMG_CHAR*, IMG_UINT32);

typedef struct PVRSRV_DC_UMJTABLE_TAG
{
    PFN_DC_MBX_CREATE_QUEUE             pfnMBXCreateCommandQueue;
    PFN_DC_MBX_REGISTER_COMMANDPROCLIST pfnMBXRegisterCmdProcList;
    PFN_DC_MBX_REMOVE_COMMANDPROCLIST   pfnMBXRemoveCmdProcList;
    PFN_DC_MBX_GET_LISR_CMDCB           pfnMBXGetLISRCommandCallback;
    PFN_DC_MBX_DESTROY_QUEUE            pfnMBXDestroyCommandQueue;
    PFN_DC_MBX_CREATE_SYNCOBJ           pfnMBXCreateSyncObj;
    PFN_DC_MBX_DESTROY_SYNCOBJ          pfnMBXDestroySyncObj;
    PFN_DC_MBX_SET_DEVICE_INFO          pfnMBXSetDeviceInfo;
    PFN_DC_MBX_SET_PVRSRV_INFO          pfnMBXSetPVRSRVInfo;
    
} PVRSRV_DC_UMJTABLE, *PPVRSRV_DC_UMJTABLE;

typedef IMG_BOOL (*PFN_DC_GET_MBXJTABLE)(PPVRSRV_DC_UMJTABLE); 


#endif 
