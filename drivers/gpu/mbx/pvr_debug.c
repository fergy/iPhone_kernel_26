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

#ifndef AUTOCONF_INCLUDED
#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
   #include <linux/modversions.h>
   #define MODVERSIONS
#endif
#endif 

#include <asm/uaccess.h>

#include <stdarg.h>
#include "img_types.h"
#include "img_defs.h"
#include "pvr_debug.h"
#include "proc.h"

#if !defined(DBG_MSG_PREFIX)
#define DBG_MSG_PREFIX "PVR_K:"
#endif

#if defined(DEBUG) || defined(TIMING)

IMG_UINT32	gPVRDebugLevel = DBGPRIV_WARNING;

#if defined(DEBUG_FUNCTION_TRACE)
const IMG_CHAR* gPVRFunctionTrace = NULL;
#endif

#define PVR_STRING_TERMINATOR		'\0'
#define PVR_IS_FILE_SEPARATOR(character) ( ((character) == '\\') || ((character) == '/') )

void PVRDebugPrintf	(
						IMG_UINT32	ui32DebugLevel,
						const IMG_CHAR*	pszFileName,
						IMG_UINT32	ui32Line,
						const IMG_CHAR*	pszFormat,
						...
					)
{
	IMG_BOOL bTrace, bDebug;
	IMG_CHAR *pszLeafName;
	
	pszLeafName = (char *)strrchr (pszFileName, '\\');
	
	if (pszLeafName)
	{
		pszFileName = pszLeafName;
	}
		
	bTrace = gPVRDebugLevel & ui32DebugLevel & DBGPRIV_CALLTRACE;
	bDebug = ((gPVRDebugLevel & DBGPRIV_ALLLEVELS) >= ui32DebugLevel);

	if (bTrace || bDebug)
	{
		va_list vaArgs;
		char szBuffer[256];

		va_start (vaArgs, pszFormat);

		
		if (bDebug)
		{
			switch(ui32DebugLevel)
			{
				case DBGPRIV_FATAL:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Fatal): ");
					break;
				}
				case DBGPRIV_ERROR:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Error): ");
					break;
				}
				case DBGPRIV_WARNING:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Warning): ");
					break;
				}
				case DBGPRIV_MESSAGE:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Message): ");
					break;
				}
				case DBGPRIV_VERBOSE:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Verbose): ");
					break;
				}
				default:
				{
					strcpy (szBuffer, DBG_MSG_PREFIX "(Unknown message level)");
					break;
				}
			}
		}
		else
		{
			strcpy (szBuffer, DBG_MSG_PREFIX );
		}

#if defined(DEBUG_FUNCTION_TRACE)
		if(gPVRFunctionTrace)
		{
			strncat(szBuffer, gPVRFunctionTrace, 25);
			strcat(szBuffer, " ");
		}
#endif

		vsprintf (&szBuffer[strlen(szBuffer)], pszFormat, vaArgs);

 		

 		if (!bTrace)
		{
			sprintf (&szBuffer[strlen(szBuffer)], " [%d, %s]", (int)ui32Line, pszFileName);
		}

		printk(KERN_INFO "%s\r\n", szBuffer);

		va_end (vaArgs);
	}
}

void PVRDebugAssertFail(const IMG_CHAR* pszFile, IMG_UINT32 uLine)
{
	PVRDebugPrintf(DBGPRIV_FATAL, pszFile, uLine, "Debug assertion failed!");
	BUG();
}

void PVRTrace(const IMG_CHAR* pszFormat, ...)
{
	static IMG_CHAR szMessage[PVR_MAX_DEBUG_MESSAGE_LEN+1];
	IMG_CHAR* pszEndOfMessage = IMG_NULL;
	va_list ArgList;

	strncpy(szMessage, DBG_MSG_PREFIX, PVR_MAX_DEBUG_MESSAGE_LEN);

	pszEndOfMessage = &szMessage[strlen(szMessage)];

	va_start(ArgList, pszFormat);
	vsprintf(pszEndOfMessage, pszFormat, ArgList);
	va_end(ArgList);

	strcat(szMessage,"\r\n");

	printk(KERN_INFO "%s", szMessage);
}


void PVRDebugSetLevel(IMG_UINT32 uDebugLevel)
{
	printk(KERN_INFO DBG_MSG_PREFIX "Setting Debug Level = 0x%x",(unsigned int)uDebugLevel);

	gPVRDebugLevel = uDebugLevel;
}

#if defined(DEBUG_FUNCTION_TRACE)
void PVRDebugSetFunctionTrace(const IMG_CHAR *pszFunctionName)
{
	gPVRFunctionTrace = pszFunctionName;
}
#endif

int PVRDebugProcSetLevel(struct file *file, const char *buffer, unsigned long count, void *data)
{
#define	_PROC_SET_BUFFER_SZ		2
	char data_buffer[_PROC_SET_BUFFER_SZ];

	PVR_UNREFERENCED_PARAMETER(file);
	PVR_UNREFERENCED_PARAMETER(data);

	if (count != _PROC_SET_BUFFER_SZ)
	{
		return -EINVAL;
	}
	else
	{
		if (copy_from_user(data_buffer, buffer, count))
			return -EINVAL;
		if (data_buffer[count - 1] != '\n')
			return -EINVAL;
		PVRDebugSetLevel(data_buffer[0] - '0');
	}
	return (count);
}

int PVRDebugProcGetLevel(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	PVR_UNREFERENCED_PARAMETER(data);

	if (off == 0) {
		*start = (char *)1;
		return printAppend(page, count, 0, "%lu\n", gPVRDebugLevel);
	}
	*eof = 1;
	return 0;
}
#endif 

#if defined(DEBUG) || defined(ENABLE_DUMP_MEM)
void PVRDebugDumpArray(unsigned int *puiData, const char *dataName, unsigned int ui32ByteSize, const char *fnName, int line)
{
	unsigned int ui32Cnt = 0;
	ui32ByteSize = (ui32ByteSize & ~3);
	printk(DBG_MSG_PREFIX "%s(%d):%s[@%08lX] of size %08ul", fnName, line, dataName, (IMG_UINT32)puiData, ui32ByteSize);
	while(ui32ByteSize)
	{
		if(ui32Cnt%8 == 0)
			printk("\n" DBG_MSG_PREFIX "    %08lX: ", (IMG_UINT32)puiData);
		else
		{
			if(ui32Cnt%4 == 0)
				printk("  ");
		}
		printk("%08X ", *puiData);
		puiData++;
		ui32ByteSize-= sizeof(unsigned int);
		ui32Cnt++;
	}
	printk("\n\n");
}
#endif 
