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

#ifndef _MBX1DEFS_
#define _MBX1DEFS_

#define MBX1_SYSREG_SIZE			0x4000
#define MBX1_FB_PCI_DECODE_SIZE		0x04000000  // 0x3B000000 //

#define MBX1_GLOBREG_PRODUCT_ID_0	(0x00000FF0)
#define MBX1_GLOBREG_PRODUCT_ID_1	(0x00000FF4)
#define MBX1_GLOBREG_PRODUCT_ID_2	(0x00000FF8)
#define MBX1_GLOBREG_PRODUCT_ID_3	(0x00000FFC)

#define MBX1_PRODUCT_ID				(0xB105F00D)
#define MBX1_LITE_PRODUCT_ID		(0xB105F00D)

#define MBX1_PART_NUMBER			(0x0910)
#define MBX1_LITE_PART_NUMBER		(0x0930)

#define MBX1_GLOBREG_PART_NUM_ID	(0x00000FE0)
#define MBX1_GLOBREG_DESIGNER_ID	(0x00000FE4)

#define MBX1_PART_NUM_1_MASK		(0x000000FF)
#define MBX1_PART_NUM_1_SHIFT		(0)
#define MBX1_PART_NUM_2_MASK		(0x0000000F)
#define MBX1_PART_NUM_2_LSHIFT		(8)

#define MBX1_GLOBREG_REVISION_ID	(0x00000FE8)
#define MBX1_GLOBREG_CORE_CONFIG	(0x00000FEC)

#define MBX1_GLOBREG_CORE_ID		(0x0F00)

#define MBX1_GROUP_ID_MASK			(0xFF000000)
#define MBX1_GROUP_ID_SHIFT			(24)

#define MBX1_GROUP_ID				(0x01)
#define MBX1_CORE_ID				(0x01)
#define MBX1_LITE_CORE_ID			(0x02)

#define MBX1_CORE_ID_MASK			(0x00FF0000)
#define MBX1_CORE_ID_SHIFT			(16)

#define MBX1_CONFIGURATION_MASK		(0x0000FFFF)
#define MBX1_CONFIGURATION_SHIFT	(0)

#define MBX1_GLOBREG_REVISION		(0x0F10)

#define MBX1_COMPANYID_MASK			(0xFF000000)
#define MBX1_COMPANYID_SHIFT		(24)

#define MBX1_VERSION_MAJOR_MASK		(0x00FF0000)
#define MBX1_VERSION_MAJOR_SHIFT	(16)

#define MBX1_VERSION_MINOR_MASK		(0x0000FF00)
#define MBX1_VERSION_MINOR_SHIFT	(8)

#define MBX1_VERSION_MAINT_MASK		(0x000000FF)
#define MBX1_VERSION_MAINT_SHIFT	(0)

#define MBX1_CONFIGURATION_VGP_PRESENT		(0x00000001)
#define MBX1_CONFIGURATION_MMU_PRESENT		(0x00000002)


#define MBX1_GLOBREG_SERIALIF_A		0x0064

#define MBX1_GLOBREG_SERIALIF_B		0x0084

#define MBX1_SERIALIF_READ_CLK		0x00000008
#define MBX1_SERIALIF_READ_DATA		0x00000004
#define MBX1_SERIALIF_CLK_CTL		0x00000002
#define MBX1_SERIALIF_DATA_CTL		0x00000001

#define MBX1_TAGLOBREG_INTSTATUS		0x012C	
#define MBX1_GLOBREG_INT_STATUS			0x012C
#define MBX1_INT_CMDPROC				0x00000001
#define MBX1_INT_RESERVED1				0x00000002
#define MBX1_INT_RENDER_COMPLETE		0x00000004
#define MBX1_INT_ISP					0x00000008
#define MBX1_INT_TA_COMPLETE			0x00000010
#define MBX1_INT_TA_OFLOW				0x00000020
#define MBX1_INT_EVM_DALLOC				0x00000040
#define MBX1_INT_TA_TIMEOUT				0x00000080
#define MBX1_INT_TA_CONTEXT				0x00000100
#define MBX1_INT_TA_STREAM_ERR			0x00000200
#define MBX1_INT_2DSYNC					0x00000400
#define MBX1_INT_MASTER					0x00008000
#define MBX1_INT_TA_FREEVCOUNT_MASK		0x00FF0000
#define MBX1_INT_TA_FREEVCOUNT_SHIFT	16
#define MBX1_INT_TA_MT_VALID			0x01000000
#define MBX1_INT_TA_MACABORT_TILE_MASK	0x7E000000
#define MBX1_INT_TA_MACABORT_TILE_SHIFT	25
#define MBX1_INT_TA_MACTILEY_SHIFT		28
#define MBX1_INT_TA_MACTILEY_MASK		0x70000000
#define MBX1_INT_TA_MACTILEX_SHIFT		25
#define MBX1_INT_TA_MACTILEX_MASK		0x0E000000

#define MBX1_INT_TA_FREE_UPPER_MASK		0x40000000	 
#define MBX1_INT_TA_FREE_UPPER_SHIFT	22

#define MBX1_INT_ALL					0x0000FFFF

#define MBX1_TAGLOBREG_INTMASK			0x0130
#define MBX1_GLOBREG_INT_MASK			0x0130
#define MBX1_INTM_RESERVED0				0x00000001
#define MBX1_INTM_RESERVED1				0x00000002
#define MBX1_INTM_RENDER_COMPLETE		0x00000004
#define MBX1_INTM_ISP					0x00000008
#define MBX1_INTM_TA_COMPLETE			0x00000010
#define MBX1_INTM_TA_OFLOW				0x00000020
#define MBX1_INTM_EVM_DALLOC			0x00000040
#define MBX1_INTM_TA_TIMEOUT			0x00000080
#define MBX1_INTM_TA_CONTEXT			0x00000100
#define MBX1_INTM_TA_STREAM_ERR			0x00000200
#define MBX1_INTM_2DSYNC				0x00000400
#define MBX1_INTM_MASTER				0x00008000

#define MBX1_TAGLOBREG_INTENABLE		0x0130	
#define MBX1_GLOBREG_INT_ENABLE			0x0130
#define MBX1_INTE_RESERVED0				0x00000001
#define MBX1_INTE_RESERVED1				0x00000002
#define MBX1_INTE_RENDER_COMPLETE		0x00000004
#define MBX1_INTE_ISP					0x00000008
#define MBX1_INTE_TA_COMPLETE			0x00000010
#define MBX1_INTE_TA_OFLOW				0x00000020
#define MBX1_INTE_EVM_DALLOC			0x00000040
#define MBX1_INTE_TA_TIMEOUT			0x00000080
#define MBX1_INTE_TA_CONTEXT			0x00000100
#define MBX1_INTE_TA_STREAM_ERR			0x00000200
#define MBX1_INTE_2DSYNC				0x00000400
#define MBX1_INTE_MASTER				0x00008000
#define MBX1_INTE_NONE					0x00000000
#define MBX1_INTE_ALL					0x000087FF


#define MBX1_INT_ENABLE(x) (MBX1_INTE_ALL & ~(x))

#define MBX1_TAGLOBREG_INTCLEAR			0x0134
#define MBX1_GLOBREG_INT_CLEAR			0x0134
#define MBX1_INTC_RESERVED0				0x00000001
#define MBX1_INTC_RESERVED1				0x00000002
#define MBX1_INTC_RENDER_COMPLETE		0x00000004
#define MBX1_INTC_ISP					0x00000008
#define MBX1_INTC_TA_COMPLETE			0x00000010
#define MBX1_INTC_TA_OFLOW				0x00000020
#define MBX1_INTC_EVM_DALLOC			0x00000040
#define MBX1_INTC_TA_TIMEOUT			0x00000080
#define MBX1_INTC_TA_CONTEXT			0x00000100
#define MBX1_INTC_TA_STREAM_ERR			0x00000200
#define MBX1_INTC_2DSYNC				0x00000400
#define MBX1_GLOBREG_SW_RESET			0x0080
#define MBX1_SW_RESET_3D				0x00000001
#define MBX1_SW_RESET_RESERVED0			0x0000000E
#define MBX1_SW_RESET_TA				0x00000010
#define MBX1_SW_RESET_RESERVED1			0x000000E0
#define MBX1_SW_RESET_BIF				0x00000100
#define MBX1_SW_RESET_RESERVED2			0xFFFFFE00
#define MBX1_SW_RESET_ALL				0x00000111

#define MBX1_GLOBREG_PART_NUM_ID	(0x00000FE0)
#define MBX1_GLOBREG_DESIGNER_ID	(0x00000FE4)
#define MBX1_GLOBREG_REVISION_ID	(0x00000FE8)

#define MBX1_GLOBREG_CORE_CONFIG	(0x00000FEC)
#define MBX1_CONFIGURATION_VGP_PRESENT	(0x00000001)
#define MBX1_CONFIGURATION_MMU_PRESENT	(0x00000002)

#endif 
