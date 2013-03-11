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

#ifndef __MALLOC_DEBUG_H__
#define __MALLOC_DEBUG_H__

#include <linux/slab.h>
#include <linux/vmalloc.h>

#if defined(DEBUG)

extern void *kmalloc_debug(size_t size, int flags, int line, const char* file);
extern void kfree_debug(const void* p, int line, const char* file);
extern void * __vmalloc_debug(unsigned long size, int gfp_mask, pgprot_t prot, int line, const char* file);
extern void * vmalloc_debug(unsigned long size, int line, const char* file);
extern void vfree_debug(void * addr, int line, const char* file);

extern void malloc_debug_init(void);
extern void malloc_debug_exit(void);

#define KMALLOC(X,Y) kmalloc_debug(X,Y, __LINE__, __FILE__)
#define KFREE(X)    kfree_debug(X, __LINE__, __FILE__)

#define __VMALLOC(X,Y,Z) __vmalloc_debug(X,Y,Z, __LINE__, __FILE__)
#define VMALLOC(X) vmalloc_debug(X, __LINE__, __FILE__)
#define VFREE(X)    vfree_debug(X, __LINE__, __FILE__)
#define MALLOC_DEBUG_INIT() malloc_debug_init()
#define MALLOC_DEBUG_EXIT() malloc_debug_exit()

#else

#define KMALLOC(X,Y) kmalloc(X,Y)
#define KFREE(X)    kfree(X)

#define __VMALLOC(X,Y,Z) __vmalloc(X,Y,Z)
#define VMALLOC(X) vmalloc(X)
#define VFREE(X)    vfree(X)

#define MALLOC_DEBUG_INIT()
#define MALLOC_DEBUG_EXIT()

#endif 


#endif 
