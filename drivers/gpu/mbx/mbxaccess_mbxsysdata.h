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

#ifndef _MBXACCESS_MBXSYSDATA_H_
#define _MBXACCESS_MBXSYSDATA_H_

#define SYS_DEVICE_COUNT 3
#define DRIVERNAME_MAXLENGTH 100

typedef struct PVRSRV_DISPLAYCLASS_INFO_TAG
{
	PVRSRV_DEVICE_CLASS                  DeviceClass;
	IMG_UINT32                           ui32DeviceID;
	PFN_SWAP_TO_SYSTEM                   pfnKSwapToSystem;
	IMG_HANDLE                           hSysSwapData;
	IMG_CHAR                             szClientDrvName[DRIVERNAME_MAXLENGTH];
	struct PVRSRV_DISPLAYCLASS_INFO_TAG *psDCInfoKM;
} PVRSRV_DISPLAYCLASS_INFO;

typedef struct PVRSRV_BUFFERCLASS_INFO_TAG
{
	PVRSRV_DEVICE_CLASS                 DeviceClass;
	IMG_UINT32                          ui32DeviceID;
	IMG_CHAR                            szClientDrvName[DRIVERNAME_MAXLENGTH];
	struct PVRSRV_BUFFERCLASS_INFO_TAG *psBCInfoKM;
} PVRSRV_BUFFERCLASS_INFO;

typedef struct _SYS_DEVICE_ID_TAG
{
	IMG_UINT32 uiID;
	IMG_BOOL   bInUse;

} SYS_DEVICE_ID;

typedef PVRSRV_ERROR (*RESMAN_FREE_FN)(IMG_UINT32 ui32ProcessID, IMG_PVOID pvParam, IMG_UINT32 ui32Param);

typedef struct _RESMAN_ITEM_
{
#if defined(DEBUG) || defined(TIMING)
	IMG_UINT32             ui32Signature;
#endif
	struct _RESMAN_ITEM_ **ppsThis; 
	struct _RESMAN_ITEM_  *psNext; 
	IMG_UINT32             ui32Flags; 
	IMG_UINT32             ui32ResType; 
	IMG_PVOID              pvParam; 
	IMG_UINT32             ui32Param; 
	RESMAN_FREE_FN         pfnFreeResource; 
	IMG_UINT32             ui32ProcessID; 
}RESMAN_ITEM, *PRESMAN_ITEM;

typedef struct _PVRSRV_DEVICE_NODE_
{
	PVRSRV_DEVICE_IDENTIFIER     sDevId;
	IMG_UINT32                   ui32RefCount;
	
	PVRSRV_ERROR               (*pfnInitDevice) (IMG_VOID*); 
	PVRSRV_ERROR               (*pfnDeInitDevice) (IMG_VOID*); 
	IMG_SYS_PHYADDR*           (*pfnGetDevSysPhysBase)(IMG_VOID*);
	IMG_VOID                   (*pfnDeviceISR)(IMG_VOID*, IMG_UINT32 ui32MBXIntEnable);
	IMG_VOID                    *pvISRData;
	
	IMG_VOID                    *pvDevice;
	IMG_UINT32                   ui32pvDeviceSize; 
	
	PRESMAN_ITEM                 psResItem;
	IMG_VOID                *pvInterruptStruct;
	struct _PVRSRV_DEVICE_NODE_ *psNext;
} PVRSRV_DEVICE_NODE;

#define QUEUE_REPROCESS_COUNT         5
#define RESMAN_TYPE_RESOURCE          0x00000001
#define RESMAN_TYPE_CMDQUEUE          0x00000020
#define RESMAN_CRITERIA_RESTYPE       0x00000001
#define RESMAN_CRITERIA_PVOID_PARAM   0x00000002
#define RESMAN_AUTOFREE_LEV1          0x10000000
#define RESMAN_AUTOFREE_LEV4          0x40000000

#if defined(OMAP2420)
#define PRCM_BASE                     0x48008000
#else
#if defined(OMAP2430)
#define PRCM_BASE                     0x49006000
#endif
#endif

#define PRCM_REG32(offset)             (offset)
#define PM_PWSTCTRL_GFX                PRCM_REG32(0x3E0)
#define PM_WKDEP_GFX                   PRCM_REG32(0x3C8)
#define PM_PWSTCTRL_GFX_ON             0x00000000
#define CM_ICLKEN_GFX                  PRCM_REG32(0x310)
#define CM_ICLKEN_GFX_EN_GFX           0x00000001
#define PM_PWSTST_GFX                  PRCM_REG32(0x3E4)
#define RM_RSTST_GFX                    PRCM_REG32(0x358)
#define PRCM_IRQSTATUS_MPU             PRCM_REG32(0x018)
#define PRCM_IRQSTATUS_MPU_FORCESTATE  0x00000021
#define CM_FCLKEN_GFX                  PRCM_REG32(0x300)
#define CM_FCLKEN_GFX_EN_3D            0x00000004
#define CM_FCLKEN_GFX_EN_2D            0x00000002
#define CM_CLKSEL_GFX                  PRCM_REG32(0x340)
#define CM_CLKSEL_GFX_L3DIV2           0x00000002
#define PRCM_CLKCFG_STATUS             PRCM_REG32(0x084)
#define PRCM_CLKCFG_STATUS_VALID       0x00000001
#define PRCM_CLKCFG_CTRL               PRCM_REG32(0x080)
#define PRCM_CLKCFG_CTRL_VALID_CONFIG  0x00000001
#define RM_RSTCTRL_GFX                 PRCM_REG32(0x350)
#define RM_RSTCTRL_GFX_RST             0x00000001

#if !defined(BM_MMU_MODE)
#define BM_MMU_MODE                    SYS_MMU_NORMAL
#endif

#define BM_DEVMEM_ARENA0_DEV_VIRT_BASE 0x00000000

#if !defined(BM_DEVMEM_ARENA0_SIZE)
#define BM_DEVMEM_ARENA0_SIZE          0x00800000
#endif

#define BM_DEVMEM_ARENA1_DEV_VIRT_BASE 0x00800000
#define BM_DEVMEM_ARENA1_SIZE          0x01800000
#define SYS_MMU_NONE                   0
#define SYS_MMU_LINEAR                 1
#define SYS_MMU_NORMAL                 2

typedef int SYS_MMU_MODE;

#endif 
