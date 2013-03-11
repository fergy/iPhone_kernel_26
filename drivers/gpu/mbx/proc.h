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

#ifndef __SERVICES_PROC_H__

#define END_OF_FILE     (off_t) -1

typedef off_t (pvr_read_proc_t)(char *, size_t, off_t);

off_t procDumpSysNodes(char *buf, size_t size, off_t off);

off_t procDumpVersion(char *buf, size_t size, off_t off);

off_t QueuePrintQueues (char * buffer, size_t size, off_t off);


int
CreateProcReadEntry (const char * name, pvr_read_proc_t handler);

off_t
printAppend (char * buffer, size_t size, off_t off, const char * format, ...)
	__attribute__((format(printf, 4, 5)));

void
RemoveProcEntry (const char * name);

void
RemoveProcEntries (void);

#endif
