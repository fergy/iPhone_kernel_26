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

#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/version.h>
#include "sysconfig.h"
#include "pvr_debug.h"
#include "mbxaccess_procwrappertype.h"
#include "mbxaccess_procwrapper_private.h"
#include "malloc_debug.h"
#include "pvrsrverror.h"
#include "mbxaccess_virtmemwrapper_private.h"

#include "pvrversion.h"

static struct proc_dir_entry * dir;


#if defined(DEBUG)
void MBXSyncSrv_PrintLineIdentifier(IMG_VOID *pvData)
{
#if defined(USE_PVR_DPF)
	PVR_DPF((PVR_DBG_MESSAGE, "%s: \n\n0x%X: \n\n", __FUNCTION__, pvData));
#else
	printk("\n\n0x%lX: \n\n", (IMG_UINT32)pvData);
#endif
}
#endif


off_t printAppend (char * buffer, size_t size, off_t off, const char * format, ...)
{
	int n;
	int space = size - off;
	off_t ret = 0;
	va_list ap;
	va_start (ap, format);
	n = vsnprintf (buffer+off, space, format, ap);
	va_end (ap);

	

	if (n > space || n < 0)
	{
		ret = size;
	}
	else
	{
		ret = off+n;
	}
	return ret;
}

static int pvr_read_proc(char *page, char **start, off_t off,
						int count, int *eof, void *data)
{
	off_t len = 0;

#ifndef DRIVERSTUB

	pvr_read_proc_t *pprn = data;

	len = pprn (page, count, off);
	if (len == END_OF_FILE)
	{
		len  = 0;
		*eof = 1;
	}
	else if (!len)
	{
		
		*start = (char *) 0;   
	}
	else
	{
		*start = (char *) 1;
	}

#else

	len = 0;

#endif

	PVR_DPF((PVR_DBG_MESSAGE,"END %s",__FUNCTION__));
	return len;
}

off_t procDumpVersion(char *buf, size_t size, off_t off)
{
	if (off == 0)
	{
		return printAppend(buf, size, 0,
							"Version %s (%s) %s %s\n",
							PVRVERSION_STRING,
							PVR_BUILD_TYPE, PVR_BUILD_DIR, PVR_BUILD_DATE);
	}
	return END_OF_FILE;
}

void MBXSyncSrv_CreateProcEntriesHandler(IMG_VOID *pvData)
{
	MBXUserDataForCreateProcEntries infoFromUser;
	COPY_FROM_USER((void*)&infoFromUser, pvData, sizeof(MBXUserDataForCreateProcEntries));
	infoFromUser.err = CreateProcEntries();
	COPY_TO_USER(pvData,(void*)&infoFromUser, sizeof(MBXUserDataForCreateProcEntries));
}

void MBXSyncSrv_RemoveProcEntriesHandler(IMG_VOID *pvData)
{
	RemoveProcEntries();
}

int CreateProcReadEntry (const char * name, void* handler)
{
	int ret;
	struct proc_dir_entry * file;

	if (!dir)
	{
		dir = proc_mkdir ("pvr", NULL);
		if(!dir)
		{
			PVR_DPF((PVR_DBG_ERROR, "cannot make proc entry /proc/pvr/%s: no parent", name));
			ret = -ENOMEM;
		}
	}
	
	file = create_proc_read_entry (name, S_IFREG | S_IRUGO, dir, pvr_read_proc, handler);

	if (file)
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
		file->owner = THIS_MODULE;
#endif
		PVR_DPF((PVR_DBG_MESSAGE, "create_proc_read_entry:FILE CREATED %s", name));
		ret = 0;
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR, "cannot make proc entry /proc/pvr/%s: no memory", name));
		ret = -ENOMEM;
	}
	
	return ret;
}

int CreateProcEntries (void)
{
	int ret = 0;
	if(!dir)
	{
		dir = proc_mkdir ("pvr", NULL);
	}

	if (!dir)
	{
		PVR_DPF((PVR_DBG_ERROR, "CreateProcEntries: cannot make /proc/pvr directory"));
		ret = -ENOMEM;
	}
	else
	{
		if(CreateProcReadEntry("version", procDumpVersion))
		{
			PVR_DPF((PVR_DBG_ERROR, "couldn't make /proc/pvr/version"));
			ret = -ENOMEM;
		}
#ifdef DEBUG
		if (CreateProcEntry ("debug_level", PVRDebugProcGetLevel, PVRDebugProcSetLevel, 0))
		{
			PVR_DPF((PVR_DBG_ERROR, "couldn't make /proc/pvr/debug_level"));
			ret = -ENOMEM;
		}
#endif
	}
	return ret;
}

int CreateProcEntry (const char * name, read_proc_t rhandler, write_proc_t whandler, void *data)
{
	int ret = 0;
	struct proc_dir_entry * file;
	mode_t mode;

	if (!dir)
	{
		PVR_DPF((PVR_DBG_ERROR, "cannot make proc entry /proc/pvr/%s: no parent", name));
		ret = -ENOMEM;
	}
	else
	{
		mode = S_IFREG;
		if (rhandler)
		{
			mode |= S_IRUGO;
		}
		if (whandler)
		{
			mode |= S_IWUSR;
		}

		file = create_proc_entry(name, mode, dir);

		if (file)
		{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
			file->owner = THIS_MODULE;
#endif
			file->read_proc = rhandler;
			file->write_proc = whandler;
			file->data = data;
			PVR_DPF((PVR_DBG_MESSAGE, "Created /proc/pvr/%s", name));
			ret = 0;
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR, "cannot make proc entry /proc/pvr/%s: no memory", name));
			ret = -ENOMEM;
		}
	}
	return ret;
}

void RemoveProcEntries (void)
{
	if(!dir)
		return;
#ifdef DEBUG
	RemoveProcEntry("debug_level");
#endif
	while (dir->subdir) {
		PVR_DPF((PVR_DBG_WARNING, "Belatedly removing /proc/pvr/%s", dir->subdir->name));
		RemoveProcEntry(dir->subdir->name);
	}
	remove_proc_entry("pvr", NULL);
	dir = NULL;
}

void RemoveProcEntry (const char *name)
{
	if (dir)
	{
		remove_proc_entry(name, dir);
	}
	PVR_DPF((PVR_DBG_MESSAGE, "Removing /proc/pvr/%s", name));
}
