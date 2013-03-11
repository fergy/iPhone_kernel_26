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

#if !defined (__SERVICESINT_H__)
#define __SERVICESINT_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include "sysinfo.h"

typedef struct _PVRSRV_REGISTRY_INFO_
{
    IMG_UINT32		ui32DevCookie;
    IMG_PCHAR		pszKey;
    IMG_PCHAR		pszValue;
    IMG_PCHAR		pszBuf;
    IMG_UINT32		ui32BufSize;
} PVRSRV_REGISTRY_INFO, *PPVRSRV_REGISTRY_INFO;

#define DRIVERNAME_MAXLENGTH	(100)
typedef struct PVRSRV_DISPLAYCLASS_INFO_TAG
{
	PVRSRV_DEVICE_CLASS					DeviceClass;
	IMG_UINT32							ui32DeviceID;
	PFN_SWAP_TO_SYSTEM					pfnKSwapToSystem;
	IMG_HANDLE							hSysSwapData;
	IMG_CHAR							szClientDrvName[DRIVERNAME_MAXLENGTH];
	struct PVRSRV_DISPLAYCLASS_INFO_TAG	*psDCInfoKM;
} PVRSRV_DISPLAYCLASS_INFO;
		
		
typedef struct PVRSRV_BUFFERCLASS_INFO_TAG
{
	PVRSRV_DEVICE_CLASS					DeviceClass;
	IMG_UINT32							ui32DeviceID;
	IMG_CHAR							szClientDrvName[DRIVERNAME_MAXLENGTH];
	struct PVRSRV_BUFFERCLASS_INFO_TAG	*psBCInfoKM;
} PVRSRV_BUFFERCLASS_INFO;
																			
#if defined (__cplusplus)
}
#endif
#endif 

