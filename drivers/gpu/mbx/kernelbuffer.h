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

#ifndef __KERNELBUFFER_H__
#define __KERNELBUFFER_H__

typedef PVRSRV_ERROR (*PFN_BC_REGISTER_BUFFER_DEV) (IMG_CHAR*, IMG_UINT32*);
typedef PVRSRV_ERROR (*PFN_BC_CREATE_QUEUE) (IMG_UINT32, PVRSRV_QUEUE_INFO **);
typedef PVRSRV_ERROR (*PFN_BC_REGISTER_COMMANDPROCLIST) (IMG_UINT32, PPFN_CMD_PROC,IMG_UINT32[][2], IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_BC_REMOVE_COMMANDPROCLIST) (IMG_UINT32, IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_BC_GET_LISR_CMDCB) (PPFN_CMD_COMPLETE);
typedef PVRSRV_ERROR (*PFN_BC_DESTROY_QUEUE) (PVRSRV_QUEUE_INFO*);
typedef PVRSRV_ERROR (*PFN_BC_REMOVE_BUFFER_DEV) (IMG_UINT32);	
typedef PVRSRV_SYNC_INFO* (*PFN_BC_CREATE_SYNCOBJ) (IMG_VOID);
typedef PVRSRV_ERROR (*PFN_BC_DESTROY_SYNCOBJ) (PVRSRV_SYNC_INFO*);

typedef struct PVRSRV_BC_KMJTABLE_TAG
{
	PFN_BC_REGISTER_BUFFER_DEV			pfnPVRSRVRegisterBufferClassDevice;
	PFN_BC_CREATE_QUEUE					pfnPVRSRVCreateCommandQueue;
	PFN_BC_REGISTER_COMMANDPROCLIST		pfnPVRSRVRegisterCmdProcList;
	PFN_BC_REMOVE_COMMANDPROCLIST		pfnPVRSRVRemoveCmdProcList;
	PFN_BC_GET_LISR_CMDCB				pfnPVRSRVGetLISRCommandCallback;
	PFN_BC_DESTROY_QUEUE				pfnPVRSRVDestroyCommandQueue;
	PFN_BC_REMOVE_BUFFER_DEV			pfnPVRSRVRemoveBufferClassDevice;
	PFN_BC_CREATE_SYNCOBJ				pfnPVRSRVCreateSyncObj;
	PFN_BC_DESTROY_SYNCOBJ				pfnPVRSRVDestroySyncObj;
} PVRSRV_BC_KMJTABLE, *PPVRSRV_BC_KMJTABLE;

typedef IMG_BOOL (*PFN_BC_GET_PVRJTABLE) (PPVRSRV_BC_KMJTABLE); 

#endif 

