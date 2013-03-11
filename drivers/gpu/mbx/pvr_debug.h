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

#ifndef __PVR_DEBUG_H__
#define __PVR_DEBUG_H__
#include "img_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define PVR_MAX_DEBUG_MESSAGE_LEN	(512)

#define DBGPRIV_FATAL	0x01
#define DBGPRIV_ERROR	0x02
#define DBGPRIV_WARNING	0x04
#define DBGPRIV_MESSAGE	0x08
#define DBGPRIV_VERBOSE	0x10
#define DBGPRIV_ALLLEVELS	(DBGPRIV_FATAL | DBGPRIV_ERROR | DBGPRIV_WARNING | DBGPRIV_MESSAGE | DBGPRIV_VERBOSE)

#define DBGPRIV_CALLTRACE 0x20


#define PVR_DBG_FATAL		DBGPRIV_FATAL,__FILE__, __LINE__
#define PVR_DBG_ERROR		DBGPRIV_ERROR,__FILE__, __LINE__
#define PVR_DBG_WARNING		DBGPRIV_WARNING,__FILE__, __LINE__
#define PVR_DBG_MESSAGE		DBGPRIV_MESSAGE,__FILE__, __LINE__
#define PVR_DBG_VERBOSE		DBGPRIV_VERBOSE,__FILE__, __LINE__
#define PVR_DBG_CALLTRACE	DBGPRIV_CALLTRACE,__FILE__, __LINE__

#if defined(DBG_MOD_NAME)
#define DBG_MSG_PREFIX "PVR_" DBG_MOD_NAME ":"
#endif

#if defined(DEBUG)
	#define PVR_ASSERT(EXPR) if (!(EXPR)) PVRDebugAssertFail(__FILE__, __LINE__);
#if defined(__linux__)
	void PVRDebugDumpArray(unsigned int *puiData, const char *dataName, unsigned int ui32ByteSize, const char *fnName, int line);
	#define PVR_DUMP_MEM(data,size)  PVRDebugDumpArray(data, #data, size, __FUNCTION__, __LINE__)
#else
	#define PVR_DUMP_MEM(data,size)
#endif

#if defined(DEBUG_FUNCTION_TRACE)
	void PVRDebugSetFunctionTrace(const IMG_CHAR *pszFunctionName);
	
	#define PVR_DPF(X)		PVRDebugSetFunctionTrace(__FUNCTION__); PVRDebugPrintf X
	#define PVR_TRACE(X)	PVRDebugSetFunctionTrace(__FUNCTION__); PVRTrace X

#else
	#define PVR_DPF(X)		PVRDebugPrintf X
	#define PVR_TRACE(X)	PVRTrace X
#endif 

void PVRDebugAssertFail	(
							const IMG_CHAR*	pszFile,
							IMG_UINT32	uLine
						);


void PVRDebugPrintf	(
						IMG_UINT32	ui32DebugLevel,
						const IMG_CHAR*	pszFileName,
						IMG_UINT32	ui32Line,
						const IMG_CHAR*	pszFormat,
						...
					);

void PVRTrace (const IMG_CHAR* pszFormat, ... );

void PVRDebugSetLevel (IMG_UINT32 uDebugLevel);

#if defined(LINUX) && defined(__KERNEL__)
#include <linux/types.h>
struct file;
int PVRDebugProcSetLevel(struct file *file, const char *buffer, unsigned long count, void *data);
int PVRDebugProcGetLevel(char *page, char **start, off_t off, int count, int *eof, void *data);
#endif

#elif defined(TIMING)
	#define PVR_ASSERT(EXPR)
	#define PVR_DPF(X)
	#define PVR_TRACE(X)	PVRTrace X
#if defined(ENABLE_DUMP_MEM)
	#define PVR_DUMP_MEM(data,size)  PVRDebugDumpArray(data, #data, size, __FUNCTION__, __LINE__)
#else
	#define PVR_DUMP_MEM(data,size)
#endif

void PVRTrace (const IMG_CHAR* pszFormat, ... );

#else
	#define PVR_ASSERT(EXPR)
	#define PVR_DPF(X)
	#define PVR_TRACE(X)
#if defined(ENABLE_DUMP_MEM)
	#define PVR_DUMP_MEM(data,size)  PVRDebugDumpArray(data, #data, size, __FUNCTION__, __LINE__)
#else
	#define PVR_DUMP_MEM(data,size)
#endif

#endif	


#if defined (__cplusplus)
}
#endif

#endif	
