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

#include "malloc_debug.h"

#if defined(DEBUG)

static    size_t total;               
static    size_t highwater;           

typedef struct ALLOC_TAG
{
	void* address;  
	size_t size;    
} allocUnit;

typedef struct LINE_LIST_TAG
{
	int line;                   
	allocUnit * allocTable;
	int tableSize;              
	int items;                  
	size_t total;               
	size_t highwater;           
	struct LINE_LIST_TAG* next;
} lineList;

typedef struct FILE_LIST_TAG
{
	const char* filename;
	lineList* lines;
	size_t total;               
	size_t highwater;           
	struct FILE_LIST_TAG* next;
} fileList;

static fileList *AllocList = 0;

static fileList* FindFileAlloc(const char* filename)
{
	fileList *item = AllocList;
	while (item)
	{
		if (item->filename == filename)
		{
			return item;
		}
		item = item->next;
	}
	
	item = kmalloc(sizeof(*item),GFP_KERNEL);
	if (!item)
	{
		printk("Out of memory\n");
		return item;
	}
	item->filename = filename;
	item->lines = NULL;
	item->total = item->highwater = 0;
	item->next = AllocList;
	AllocList = item;
	return item;
}

static lineList* FindLineAlloc(fileList *fList, int line)
{
	lineList *item = fList->lines;
	while (item)
	{
		if (item->line == line)
		{
			return item;
		}
		item = item->next;
	}
	
	item = kmalloc(sizeof(*item),GFP_KERNEL);
	if (!item)
	{
		printk("Out of memory\n");
		return item;
	}
	item->line = line;
	item->allocTable = NULL;
	item->tableSize = 0;
	item->items = 0;
	item->total = item->highwater = 0;
	item->next = fList->lines;
	fList->lines = item;
	return item;
}

#define GRANULARITY 5

static void AllocTableInsert(lineList* lList, size_t size, void* address)
{
	if (lList->items == lList->tableSize)
	{
		
		int newsize = lList->tableSize + GRANULARITY;
		allocUnit* table = kmalloc(newsize * sizeof(allocUnit), GFP_KERNEL);
		if (!table)
		{
			printk("Out of memory\n");
			return;
		}
		if (lList->allocTable)
		{
			memcpy(table, lList->allocTable, lList->tableSize * sizeof(allocUnit));
			kfree(lList->allocTable);
		}
		lList->allocTable = table;
		lList->tableSize = newsize;
	}
	lList->allocTable[lList->items].address = address;
	lList->allocTable[lList->items].size = size;
	++lList->items;
	lList->total += size;
	if (lList->highwater < lList->total)
	{
		lList->highwater = lList->total;
	}
}

static size_t FindAndRemove(lineList* lList, const void* address)
{
	int i;
	for(i=0; i< lList->items; i++)
	{
		if (lList->allocTable[i].address == address)
		{
			size_t size = lList->allocTable[i].size;
			lList->total -= size;
			--lList->items;
			if (i != lList->items )
			{
				lList->allocTable[i] = lList->allocTable[lList->items];
			}
			return size;
		}
	}
	return 0;
}

static void Insert(size_t size, void* address, int line, const char* file)
{
	lineList* lList;
	fileList* fList = FindFileAlloc(file);
	if (!fList)
	{
		return;
	}
	lList = FindLineAlloc(fList,line);
	if (!lList)
	{
		return;
	}
	AllocTableInsert(lList, size, address);
	fList->total += size;
	if (fList->highwater < fList->total)
	{
		fList->highwater = fList->total;
	}
	total += size;
	if (highwater < total)
	{
		highwater = total;
	}
}

static void Remove(const void* address, int line, const char* file)
{
	size_t size;
	fileList* fList = AllocList;
	while(fList)
	{
		lineList* lList = fList->lines;
		while(lList)
		{
			size = FindAndRemove(lList, address);
			if (size)
			{
				fList->total -= size;
				total -=size;
				return;
			}
			lList = lList->next;
		}
		fList = fList->next;
	}
	printk("Address %p not found in allocation list %i,%s\n", address, line, file);
}

static void Cleanup(int debug)
{
	fileList* fList;
	lineList* lList;
	
	if (debug)
	{
		printk("/**************** kmalloc/vmalloc stats **********************/\n");
		if (total)
		{
			printk("High water %u B, still allocated %u B\n", highwater, total);
		}
		else
		{
			printk("High water %u B\n", highwater);
		}
	}
	fList = AllocList;
	while(fList)
	{
		fileList* filenext = fList->next;
		if (debug)
		{
			if (fList->total)
			{
				printk("%s: high water %u B, still allocated %u B\n", fList->filename, fList->highwater, fList->total);
			}
			else
			{
				printk("%s: high water %u B\n", fList->filename, fList->highwater);
			}
		}
		lList = fList->lines;
		while(lList)
		{
			lineList* linenext = lList->next;
			if (debug)
			{
				if (lList->total)
				{
					printk("\tLine %i: high water %u B, still allocated %u B in %i segments\n", lList->line, lList->highwater, lList->total, lList->items);
				}
				else
				{
					printk("\tLine %i: high water %u B\n", lList->line, lList->highwater);
				}
			}

			if (lList->allocTable)
			{
				if (debug)
				{
					int i;
					for(i=0;i<lList->items;i++)
					{
						printk("\t\t%p,\t%u B\n", lList->allocTable[i].address, lList->allocTable[i].size);
					}
				}
				kfree(lList->allocTable);
			}
			kfree(lList);
			lList = linenext;
		}
		kfree(fList);
		fList = filenext;
	}
	AllocList = NULL;
	if (debug)
	{
		printk("/*************************************************************/\n");
	}
}

void PrintAllocInfo(int detailed)
{
	lineList* lList;
	fileList* fList = AllocList;
	while(fList)
	{
		if (fList->total)
		{
			printk("%s: %u bytes\n", fList->filename, fList->total);
			lList = fList->lines;
			while(lList)
			{
				if (lList->items)
				{
					printk("\tLine:\t%i,\tallocs:\t%i,\ttotal size\t%u B\n", lList->line, lList->items, lList->total);
					if (detailed)
					{
						int i;
						for(i=0; i<lList->items; i++)
						{
							printk("\t\t%p,\t%u B\n", lList->allocTable[i].address, lList->allocTable[i].size);
						}
					}
				}
				lList = lList->next;
			}
		}
		fList = fList->next;
	}
}

void *kmalloc_debug(size_t size, int flags, int line, const char* file)
{
	void * address = kmalloc(size, flags);
	if (!address)
	{
		printk("ERROR: kmalloc %u bytes at %i,%s failed\n",size, line, file);
		return address;
	}
	Insert(size, address, line, file);
	return address;
}

void kfree_debug(const void* p, int line, const char* file)
{
	Remove(p, line, file);
	kfree(p);
}

void * __vmalloc_debug(unsigned long size, int gfp_mask, pgprot_t prot, int line, const char* file)
{
	void * address = __vmalloc(size, gfp_mask, prot);
	if (!address)
	{
		printk("ERROR: __vmalloc %lu bytes at %i,%s failed\n",size, line, file);
		return address;
	}
	Insert(size, address, line, file);
	return address;
}

void * vmalloc_debug(unsigned long size, int line, const char* file)
{
	void * address = vmalloc(size);
	if (!address)
	{
		printk("ERROR: __vmalloc %lu bytes at %i,%s failed\n",size, line, file);
		return address;
	}
	Insert(size, address, line, file);
	return address;
}

void vfree_debug(void * addr, int line, const char* file)
{
	Remove(addr, line, file);
	vfree(addr);
}

void malloc_debug_init(void)
{
	AllocList = NULL;
	total = highwater = 0;
}

void malloc_debug_exit(void)
{
	Cleanup(1);
}
#endif 
