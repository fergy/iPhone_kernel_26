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

#ifndef __SYSCONFIG_H__
#define __SYSCONFIG_H__

#define SYS_MAX_PAGES				1024
#define PVR_BUILD_TYPE "release"
#define PVR_BUILD_DIR "/root"
#define PVR_BUILD_DATE "today"
#define LINUX
#define MBXACCESS_DEVICE "mbx"
#define VS_PRODUCT_NAME	"IPHONE 3G"

#if !defined(DISABLE_TA)

#define MBX_REG_SYS_PHYS_BASE				0x3B000000	
#define MBX_REG_RANGE						0x00004000

#define MBX_SP_3D_SYS_PHYS_BASE				0x3B008000	
#define MBX_SP_3D_RANGE						0x00002000

#define MBX_SP_2D_SYS_PHYS_BASE				0x3B00A000	
#define MBX_SP_2D_RANGE						0x00002000

#define MBX_SP_TA_CTRL_SYS_PHYS_BASE		0x3B00C000	
#define MBX_SP_TA_CTRL_RANGE				0x00002000

#else

#define MBX_REG_SYS_PHYS_BASE				0x3B000000	
#define MBX_REG_RANGE						0x00002000

#define MBX_SP_3D_SYS_PHYS_BASE				0x3B002000	
#define MBX_SP_3D_RANGE						0x00001000

#define MBX_SP_2D_SYS_PHYS_BASE				0x3B00A000	
#define MBX_SP_2D_RANGE						0x00002000

#define MBX_SP_TA_CTRL_SYS_PHYS_BASE		0x3B003000	
#define MBX_SP_TA_CTRL_RANGE				0x00001000

#endif


#define MBX_ADDRESSABLE_FB_SYS_PHYS_BASE	0x00000000
#define MBX_ADDRESSABLE_FB_SIZE				0x02000000	


#if defined(LINUX)

#define BM_NUM_CONTIG_POOLS 				0
#define BM_NUM_DEVMEM_ARENAS				2

#define BM_MMU_MODE							SYS_MMU_NORMAL

#define BM_DEVMEM_ARENA0_DEV_VIRT_BASE		0x00000000
#define BM_DEVMEM_ARENA0_SIZE				0x00800000	
	
#define BM_DEVMEM_ARENA1_DEV_VIRT_BASE		0x00800000
#define BM_DEVMEM_ARENA1_SIZE				0x01800000	

#else

#error "Must define BM pools for your environment"

#endif

#if defined(TIMING)
#define SOC_REG_BASE                        0x4808a000 //??
#else
#define SOC_REG_BASE                        0
#endif
#define SOC_REG_RANGE                       0x00001000 

#endif	

