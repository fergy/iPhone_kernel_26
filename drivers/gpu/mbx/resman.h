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

#ifndef __RESMAN_H__
#define __RESMAN_H__

#if defined (__cplusplus)
extern "C" {
#endif

#define RESMAN_TYPE_RESOURCE			0x00000001	
#define RESMAN_TYPE_FRAMBUFMEM			0x00000002	
#define RESMAN_TYPE_PROCESS_TERM		0x00000004	
#define RESMAN_TYPE_RENDERTARGET        0x00000008	
#define RESMAN_TYPE_PARAMBUFF           0x00000010	
#define RESMAN_TYPE_CMDQUEUE			0x00000020	
#define RESMAN_TYPE_BLITCLIENT			0x00000040	
#define RESMAN_TYPE_SYSTEM_SURFACE		0x00000080	
#define RESMAN_TYPE_SHARED_SYNCOBJECTS	0x00000100	
#define RESMAN_TYPE_TIMER			0x00000200	
#define RESMAN_TYPE_USE_PROCESSID		0x80000000	

#define RESMAN_CRITERIA_ALL				0x00000000	
#define RESMAN_CRITERIA_RESTYPE			0x00000001	
#define RESMAN_CRITERIA_PVOID_PARAM		0x00000002	
#define RESMAN_CRITERIA_UI32_PARAM		0x00000004	
#define RESMAN_CRITERIA_AUTOFREE		0x00000008	
#define RESMAN_CRITERIA_FREEONCE        0x00000010	
#define RESMAN_CRITERIA_CLIENT_ID		0x00000020	

#define RESMAN_PROCESSID_FIND			0xffffffff	

#define RESMAN_AUTOFREE_OFF				0x00000000	
#define RESMAN_AUTOFREE_LEV1			0x10000000	
#define RESMAN_AUTOFREE_LEV2			0x20000000	
#define RESMAN_AUTOFREE_LEV3			0x30000000	
#define RESMAN_AUTOFREE_LEV4			0x40000000	
#define RESMAN_AUTOFREE_LEV5			0x50000000	
#define RESMAN_AUTOFREE_LEV6			0x60000000	
#define RESMAN_AUTOFREE_LEV7			0x70000000	


#define RESMAN_FLAGS_CALLER_ALLOCED_BUF		0x00000001	
#define RESMAN_FLAGS_AUTOFREE_MASK			0xf0000000	

typedef PVRSRV_ERROR (*RESMAN_FREE_FN)(IMG_UINT32 ui32ProcessID, IMG_PVOID pvParam, IMG_UINT32 ui32Param);
																		


typedef struct _RESMAN_ITEM_
{
#if defined(DEBUG) || defined(TIMING)
	IMG_UINT32				ui32Signature;
#endif
	struct _RESMAN_ITEM_	**ppsThis;	
	struct _RESMAN_ITEM_	*psNext;	

	IMG_UINT32				ui32Flags;	
	IMG_UINT32				ui32ResType;

	IMG_PVOID				pvParam;	
	IMG_UINT32				ui32Param;	

	RESMAN_FREE_FN			pfnFreeResource;
	int					clientID;

	IMG_UINT32				ui32ProcessID;

}RESMAN_ITEM, *PRESMAN_ITEM;


typedef struct _RESMAN_PROCESS_
{
#if defined(DEBUG) || defined(TIMING)
	IMG_UINT32					ui32Signature;
#endif
	struct	_RESMAN_PROCESS_	**ppsThis;
	struct	_RESMAN_PROCESS_	*psNext;

	IMG_UINT32					ui32ProcessID;
	IMG_UINT32					ui32RefCount; 
	RESMAN_ITEM					*psResItemList;

}RESMAN_PROCESS, *PRESMAN_PROCESS;


typedef struct
{
	RESMAN_PROCESS	*psProcessList;

} RESMAN_LIST, *PRESMAN_LIST;


PVRSRV_ERROR ResManProcessConnect(IMG_UINT32 ui32ProcID);
PVRSRV_ERROR ResManProcessDisconnect(IMG_UINT32 ui32ProcID);
RESMAN_ITEM *ResManRegisterRes(RESMAN_ITEM		*psResItem2Use,
							   IMG_UINT32		ui32ResType,
							   IMG_PVOID		pvParam,
							   IMG_UINT32		ui32Param,
								int		clientID,
							   RESMAN_FREE_FN	pfnFreeResource,
							   IMG_UINT32		ui32AutoFreeLev,
							   IMG_UINT32		ui32ProcessID);

PVRSRV_ERROR ResManFreeResByPtr(RESMAN_ITEM	*psResItem,
								IMG_BOOL	bExecuteCallback);

PVRSRV_ERROR ResManFreeResByCriteria(IMG_UINT32	ui32SearchCriteria,
									IMG_UINT32	ui32ResType,
									IMG_PVOID	pvParam,
									IMG_UINT32	ui32Param,
									int		clientID,
									IMG_UINT32	ui32AutoFreeLev,
									IMG_BOOL	bExecuteCallback);

PVRSRV_ERROR ResManPrePower(PVR_POWER_STATE eNewPowerState,
							PVR_POWER_STATE eCurrentPowerState);

PVRSRV_ERROR ResManPostPower(PVR_POWER_STATE eNewPowerState,
							 PVR_POWER_STATE eCurrentPowerState);

PVRSRV_ERROR FreeDeviceMemCallBack(IMG_UINT32	ui32ProcessID,
								   IMG_PVOID	pvParam,
								   IMG_UINT32	ui32Param);

#if defined (__cplusplus)
}
#endif

#endif 

