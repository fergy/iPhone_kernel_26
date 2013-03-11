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

#ifndef __PVRSRVERROR_H__
#define __PVRSRVERROR_H__

typedef enum _PVRSRV_ERROR_
{
	PVRSRV_OK							=  0,
	PVRSRV_ERROR_GENERIC				=  1,
	PVRSRV_ERROR_OUT_OF_MEMORY			=  2,
	PVRSRV_ERROR_TOO_MANY_BUFFERS		=  3,
	PVRSRV_ERROR_SYMBOL_NOT_FOUND		=  4,
	PVRSRV_ERROR_OUT_OF_HSPACE			=  5,
	PVRSRV_ERROR_INVALID_PARAMS			=  6,
	PVRSRV_ERROR_TILE_MAP_FAILED		=  7,
	PVRSRV_ERROR_INIT_FAILURE			=  8,
	PVRSRV_ERROR_CANT_REGISTER_CALLBACK =  9,
	PVRSRV_ERROR_INVALID_DEVICE			= 10,
	PVRSRV_ERROR_NOT_OWNER				= 11,
	PVRSRV_ERROR_BAD_MAPPING			= 12,
	PVRSRV_ERROR_TIMEOUT				= 13,
	PVRSRV_ERROR_NO_PRIMARY				= 14,
	PVRSRV_ERROR_FLIP_CHAIN_EXISTS		= 15,
	PVRSRV_ERROR_CANNOT_ACQUIRE_SYSDATA = 16,
	PVRSRV_ERROR_SCENE_INVALID			= 17,
	PVRSRV_ERROR_STREAM_ERROR			= 18,
	PVRSRV_ERROR_INVALID_INTERRUPT      = 19,
	PVRSRV_ERROR_FAILED_DEPENDENCIES	= 20,
	PVRSRV_ERROR_CMD_NOT_PROCESSED		= 21,
	PVRSRV_ERROR_CMD_TOO_BIG			= 22,
	PVRSRV_ERROR_DEVICE_REGISTER_FAILED = 23,
	PVRSRV_ERROR_FIFO_SPACE				= 24,
	PVRSRV_ERROR_TA_RECOVERY			= 25,
	PVRSRV_ERROR_BRIDGE_LOCKED			= 27,
	PVRSRV_ERROR_TA_LOCKED				= 28,
	PVRSRV_ERROR_FORCE_I32 = 0x7fffffff
} PVRSRV_ERROR;

#endif
