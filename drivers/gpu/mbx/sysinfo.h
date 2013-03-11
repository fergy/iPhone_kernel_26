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

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#define MAX_HW_TIME_US				(500000)
#define WAIT_TRY_COUNT				(10000)

#if defined (__cplusplus)
extern "C" {
#endif
extern IMG_UINT32 gui32MBXDeviceID;

typedef enum _SYS_DEVICE_TYPE_
{
	
	SYS_DEVICE_MBX						= 1,

	SYS_DEVICE_FORCE_I16 				= 0x7fff

} SYS_DEVICE_TYPE;

typedef enum _PVRSRV_CLOCK_CORE_TYPE_
{
	PVRSRV_CLOCK_CORE_UNKNOWN			= 0,
	PVRSRV_CLOCK_CORE_MBX_3D			= 1,
	PVRSRV_CLOCK_CORE_MBX_2D			= 2,
	
	PVRSRV_CLOCK_CORE_FORCE_I32			= 0x7fffffff

} PVRSRV_CLOCK_CORE_TYPE;


#define SYS_DEVICE_COUNT 3 

typedef enum _MBX_COMMAND_TYPE_
{
	MBX_RENDER_COMMAND					=  0,
	MBX_BLT_COMMAND,

#if defined (SUPPORT_3D_BLIT)
	MBX_3D_BLT_COMMAND,
#endif

	MBX_COMMAND_COUNT,
	PVRSRV_ERROR_FORCE_I16 				=  0x7fff

} MBX_COMMAND_TYPE;

#define MBX1_SP_FIFO_DWSIZE         59

#define MBX1_SP_FIFO_RESERVEBYTES   	(MBX1_SP_FIFO_DWSIZE & -4)
#define MBX1_SP_FIFO_MAXALLOWEDBYTES	(MBX1_SP_FIFO_DWSIZE * 4) - MBX1_SP_FIFO_RESERVEBYTES

#define MBX_EXTRACT_FIFO_COUNT(x)   (((x) & MBX1_INT_TA_FREEVCOUNT_MASK) >> MBX1_INT_TA_FREEVCOUNT_SHIFT)

#define SYS_KICKER_VALUE		MBX1_INT_ISP

#if !defined(PRCM_BASE)

#if defined(CONFIG_IPHONE_3G)

#define PRCM_BASE             0x48008000

#else
//FIXME!!///////////////////////////////////
#if defined(CONFIG_IPHONE_3GS)

#define PRCM_BASE             0x49006000
////////////////////////////////////////////
#else

#if defined(LINUX)
#error("ARM_MACH-IPHONE platform undefined")
#endif

#endif 
#endif 
#endif 


#if !defined(PRCM_REG32)
#define PRCM_REG32(offset)	  (offset)
#endif

#define PRCM_SYSCONFIG        PRCM_REG32(0x010)
#define 	PRCM_SYSCONFIG_AUTOUIDLE			0x00000001

#define PRCM_IRQSTATUS_MPU    PRCM_REG32(0x018)
#define 	PRCM_IRQSTATUS_MPU_FORCESTATE		0x00000021

#define PRCM_CLKCFG_CTRL      PRCM_REG32(0x080)
#define 	PRCM_CLKCFG_CTRL_VALID_CONFIG		0x00000001

#define PRCM_CLKCFG_STATUS    PRCM_REG32(0x084)
#define 	PRCM_CLKCFG_STATUS_VALID			0x00000001

#define CM_FCLKEN_GFX         PRCM_REG32(0x300)
#define 	CM_FCLKEN_GFX_EN_3D					0x00000004
#define 	CM_FCLKEN_GFX_EN_2D					0x00000002

#define CM_ICLKEN_GFX         PRCM_REG32(0x310)
#define 	CM_ICLKEN_GFX_EN_GFX				0x00000001

#define CM_IDLEST_GFX         PRCM_REG32(0x320)
#define 	CM_IDLEST_GFX_ST_GFX				0x00000001

#define CM_CLKSEL_GFX         PRCM_REG32(0x340)
#define 	CM_CLKSEL_GFX_MASK					0x0000000f
#define 	CM_CLKSEL_GFX_L3DIV1				0x00000001
#define 	CM_CLKSEL_GFX_L3DIV2				0x00000002
#define 	CM_CLKSEL_GFX_L3DIV3				0x00000003
#define 	CM_CLKSEL_GFX_L3DIV4				0x00000004

#define CM_CLKSTCTRL_GFX      PRCM_REG32(0x348)
#define 	CM_CLKSTCTRL_GFX_AUTOSTATE			0x00008001

#define RM_RSTCTRL_GFX        PRCM_REG32(0x350)
#define 	RM_RSTCTRL_GFX_RST					0x00000001

#define RM_RSTST_GFX          PRCM_REG32(0x358)
#define 	RM_RSTST_GFX_SW_RST					0x00000008
#define 	RM_RSTST_GFX_DOMAINWKUP_RST			0x00000002

#define PM_WKDEP_GFX          PRCM_REG32(0x3C8)
#define 	PM_WKDEP_GFX_EN_WAKEUP				0x00000010
#define 	PM_WKDEP_GFX_EN_MPU					0x00000002
#define 	PM_WKDEP_GFX_EN_CORE				0x00000001

#define PM_PWSTCTRL_GFX       PRCM_REG32(0x3E0)
#define 	PM_PWSTCTRL_GFX_FORCESTATE			0x00040000
#define 	PM_PWSTCTRL_GFX_MEMONSTATE_MASK		0x00000c00
#define 	PM_PWSTCTRL_GFX_MEMRETSTATE			0x00000008
#define 	PM_PWSTCTRL_GFX_LOGICRETSTATE		0x00000004
#define 	PM_PWSTCTRL_GFX_POWERSTATE_MASK		0x00000003
#define 		PM_PWSTCTRL_GFX_OFF				0x00000003
#define 		PM_PWSTCTRL_GFX_RETENTION		0x00000001
#define 		PM_PWSTCTRL_GFX_ON				0x00000000

#define PM_PWSTST_GFX         PRCM_REG32(0x3E4)
#define 	PM_PWSTST_GFX_INTRANSITION			0x00100000
#define 	PM_PWSTST_GFX_CLKACTIVITY			0x00080000
#define 	PM_PWSTST_GFX_POWERSTATE_MASK		0x00000003
#define 		PM_PWSTST_GFX_OFF				0x00000003
#define 		PM_PWSTST_GFX_RETENTION			0x00000001
#define 		PM_PWSTST_GFX_ON				0x00000000

#define 	MBX1_GLOBREG_GPO					0x00000108
#define 	MBX1_GLOBREG_GPO_3D_CLK_GATING_EN	0x00000010
#define 	MBX1_GLOBREG_GPO_2D_CLK_GATING_EN	0x00000008

IMG_IMPORT 
IMG_VOID SysKickCmdProc(IMG_UINT32 *pui32KickerAddr);

#if defined(PVR_KERNEL)
PVRSRV_ERROR SysCoreControlKM(IMG_VOID *pvClkCtrlRegBase, 
							  PVRSRV_CLOCK_CORE_TYPE eCoreType,
							  IMG_BOOL bEnable, 
							  IMG_BOOL bBlock);
#else
PVRSRV_ERROR SysCoreControl(IMG_VOID *pvClkCtrlRegBase, 
							PVRSRV_CLOCK_CORE_TYPE eCoreType,
							IMG_BOOL bEnable, 
							IMG_BOOL bBlock);
#endif
#if defined(__cplusplus)
}
#endif

#endif	

