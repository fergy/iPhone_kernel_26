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

#ifndef __IMG_DEFS_H__
#define __IMG_DEFS_H__


#include "img_types.h"

typedef	enum img_tag_TriStateSwitch
{
	IMG_ON = 0x00,
	IMG_OFF,
	IMG_IGNORE

} img_TriStateSwitch, * img_pTriStateSwitch;

#define IMG_SUCCESS			0

#define IMG_NULL			0
#define IMG_NO_REG			1


#if !defined (INLINE)
#if defined (NO_INLINE_FUNCS)
#define INLINE
#else
#if defined (__cplusplus)
#define INLINE	inline
#else
#define INLINE	__inline
#endif
#endif
#endif 

#if !defined (FORCE_INLINE)
#if defined (NO_INLINE_FUNCS)
#define FORCE_INLINE
#else
#if defined (__cplusplus)
#define FORCE_INLINE	inline
#else
#define FORCE_INLINE	__inline
#endif
#endif
#endif 

#ifndef PVR_UNREFERENCED_PARAMETER
#define PVR_UNREFERENCED_PARAMETER(param) (param) = (param)
#endif

#ifdef __GNUC__
#define unref__ __attribute__ ((unused))
#else
#define unref__
#endif

typedef char	TCHAR, *PTCHAR, *PTSTR;

#define IMG_CALLCONV 
#define IMG_IMPORT
#define IMG_EXPORT
#define IMG_INTERNAL	__attribute__ ((visibility ("hidden")))
#define IMG_EXTERNAL
#define APIENTRY

#endif 

