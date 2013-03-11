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

#if !defined (__KERNELDISPLAY_H__)
#define __KERNELDISPLAY_H__


typedef PVRSRV_ERROR (*PFN_DC_REGISTER_DISPLAY_DEV)(	PFN_SWAP_TO_SYSTEM,	IMG_HANDLE, IMG_CHAR*, IMG_UINT32*);

typedef PVRSRV_ERROR (*PFN_DC_CREATE_QUEUE) (IMG_UINT32, PVRSRV_QUEUE_INFO **);

typedef PVRSRV_ERROR (*PFN_DC_REGISTER_COMMANDPROCLIST) (IMG_UINT32, PPFN_CMD_PROC,IMG_UINT32[][2], IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_DC_REMOVE_COMMANDPROCLIST) (IMG_UINT32, IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_DC_GET_LISR_CMDCB) (PPFN_CMD_COMPLETE);
typedef PVRSRV_ERROR (*PFN_DC_DESTROY_QUEUE) (PVRSRV_QUEUE_INFO*);
typedef PVRSRV_ERROR (*PFN_DC_REMOVE_DISPLAY_DEV)	(IMG_UINT32);	
typedef PVRSRV_SYNC_INFO* (*PFN_DC_CREATE_SYNCOBJ)	(IMG_VOID);
typedef PVRSRV_ERROR (*PFN_DC_DESTROY_SYNCOBJ)	(PVRSRV_SYNC_INFO*);
typedef PVRSRV_ERROR (*PFN_DC_REGISTER_SYS_ISR)	(PFN_ISR_HANDLER, IMG_VOID*, IMG_UINT32, IMG_UINT32);
typedef PVRSRV_ERROR (*PFN_DC_OEM_FUNCTION)	(IMG_UINT32, IMG_VOID*, IMG_VOID*);

typedef struct PVRSRV_DC_KMJTABLE_TAG
{
	PFN_DC_REGISTER_DISPLAY_DEV		pfnPVRSRVRegisterDisplayClassDevice;
	PFN_DC_CREATE_QUEUE				pfnPVRSRVCreateCommandQueue;
	PFN_DC_REGISTER_COMMANDPROCLIST	pfnPVRSRVRegisterCmdProcList;
	PFN_DC_REMOVE_COMMANDPROCLIST	pfnPVRSRVRemoveCmdProcList;
	PFN_DC_GET_LISR_CMDCB			pfnPVRSRVGetLISRCommandCallback;
	PFN_DC_DESTROY_QUEUE			pfnPVRSRVDestroyCommandQueue;
	PFN_DC_REMOVE_DISPLAY_DEV		pfnPVRSRVRemoveDisplayClassDevice;
	PFN_DC_CREATE_SYNCOBJ			pfnPVRSRVCreateSyncObj;
	PFN_DC_DESTROY_SYNCOBJ			pfnPVRSRVDestroySyncObj;
	PFN_DC_REGISTER_SYS_ISR			pfnPVRSRVRegisterSystemISRHandler;
	PFN_DC_OEM_FUNCTION				pfnPVRSRVOEMFunction;
} PVRSRV_DC_KMJTABLE, *PPVRSRV_DC_KMJTABLE;

typedef IMG_BOOL (*PFN_DC_GET_PVRJTABLE) (PPVRSRV_DC_KMJTABLE); 

#endif
