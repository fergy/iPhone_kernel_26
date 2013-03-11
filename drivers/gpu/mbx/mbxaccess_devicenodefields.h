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

#ifndef __MBXACCESS_DEVICE_NODE_FIELDS_H__
#define __MBXACCESS_DEVICE_NODE_FIELDS_H__

typedef enum MBXDeviceNodeFieldEnum
{
	 MBXDEVICENODEFIELD_DEVICE_NODE = 0X0000,
	 MBXDEVICENODEFIELD_DEVICE_TYPE,
	 MBXDEVICENODEFIELD_DEVICE_CLASS,
	 MBXDEVICENODEFIELD_DEVICE_INDEX,
	 MBXDEVICENODEFIELD_DEVICE_INDEX_LIST_COUNT,
	 MBXDEVICENODEFIELD_DEVICE_INDEX_LIST,
	 MBXDEVICENODEFIELD_REF_COUNT,
	 MBXDEVICENODEFIELD_PFN_DEVICE_INIT,
	 MBXDEVICENODEFIELD_PFN_DEVICE_DEINIT,
	 MBXDEVICENODEFIELD_PFN_DEVICE_PHYS_BASE,
	 MBXDEVICENODEFIELD_PFN_DEVICE_ISR,
	 MBXDEVICENODEFIELD_DEVICE_ISR_DATA,
	 MBXDEVICENODEFIELD_DEVICE_INFO,
     MBXDEVICENODEFIELD_DEVICE_INFO_US,
     MBXDEVICENODEFIELD_DEVICE_SIZE,
     MBXDEVICENODEFIELD_DEVICE_RES_ITEM,
	 MBXDEVICENODEFIELD_DEVICE_INFO_DISPLAY,
	 MBXDEVICENODEFIELD_DEVICE_INTERRUPT_STRUCT,
	 MBXDEVICENODEFIELD_DEVICE_PVRSRV_INFO_DISPLAY,
	 MBXDEVICENODEFIELD_DEVICE_PVRSRV_DEVICE_DELETE,
	 MBXDEVICENODEFIELD_DEVICE_RESMAN_INFO,
	 MBXDEVICENODEFIELD_END = 0XFFFF
} MBXDeviceNodeField;

#endif 

