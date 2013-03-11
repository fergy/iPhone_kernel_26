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
#endif

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
   #include <config/modversions.h>
   #define MODVERSIONS
#endif

#include <linux/version.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#include <linux/wrapper.h>
#endif

#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/shmparam.h>


#include "services_headers.h"

#if defined(DO_TLB_FLUSH_ON_MMAP)
#include <asm/tlbflush.h>
#endif

#include "img_defs.h"
#include "services.h"
#include "pvrmmap.h"
#include "pvrmmap_private.h"
#include "mbxaccess_virtmemwrapper_private.h"
#include "mmap.h"

#include "pvr_debug.h"
#include "hostfunc.h"
#include "proc.h"
#include "env.h"
#include "mutex.h"


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
  #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_page_range(dst,src,size,prot))
  #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_page_range(dst,page_to_phys(src),size,prot))
#else
  #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
  
    #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_page_range(vma,dst,src,size,prot))
    #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_page_range(vma,dst,page_to_phys(src),size,prot))
  #else
  
    #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_pfn_range(vma,dst,(src>>PAGE_SHIFT),size,prot))
    #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_pfn_range(vma,dst,(page_to_pfn(src)>>PAGE_SHIFT),size,prot))
  #endif
#endif

static IMG_UINT32 GetFirstFreePageAlignedNumber(void);
static PKV_OFFSET_STRUCT FindOffsetStructBy_MEM_AREA(PMEM_AREA psLinuxMemArea);

static PKV_OFFSET_STRUCT FindOffsetStructureByOffset(IMG_UINT32 ui32Offset);
static PKV_OFFSET_STRUCT FindOffsetStructureByKVAddr(IMG_VOID *pvVirtAddress);

#define	COLOUR_MASK					((SHMLBA - 1) >> PAGE_SHIFT)

#define	DCACHE_COLOUR(vaddr)		(((vaddr) >>  PAGE_SHIFT) & COLOUR_MASK)

#define	COLOUR_ALIGN(addr,pgoff)	((((addr) + SHMLBA - 1) & ~(SHMLBA - 1)) +  \
										(((pgoff) << PAGE_SHIFT) & (SHMLBA - 1)))


static void PVRMMapVOpen(struct vm_area_struct* ps_vma);
static void PVRMMapVClose(struct vm_area_struct* ps_vma);
static IMG_UINT32 GetProcessID(IMG_VOID);

static struct vm_operations_struct PVRMMapVMOps =
{
	open:		PVRMMapVOpen,
	close:		PVRMMapVClose,
};

extern PVRSRV_LINUX_MUTEX gPVRSRVLock;

static PKV_OFFSET_STRUCT psKVOffsetTable = 0;
static LinuxKMemCache *psMemmapCache = 0;
static IMG_UINT32 ui32RegisteredAreas = 0;


static IMG_UINT32 GetProcessID(IMG_VOID)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	return (IMG_UINT32)current->pgrp;
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
	return (IMG_UINT32)task_tgid_nr(current);
#else
	return (IMG_UINT32)current->tgid;
#endif
#endif
}


static off_t
PrintVirtualMemRegistrations (char * buffer, size_t size, off_t off)
{
	PKV_OFFSET_STRUCT pv;
	
	if (!off)
		return printAppend (buffer, size, 0,
							"Allocations registered for mmap: %lu\n"
#if defined(DEBUG_HOST_ALLOC)
							"@                 "
#endif
							"Addr       Offset   Length Type Phys       Cache WriteC Pid Mapped Name\n"
							,ui32RegisteredAreas);
	if (size < 80) 
		return 0;
	
	for (pv = psKVOffsetTable; --off && pv; pv = pv->psNext)
		;
	if (!pv)
		return END_OF_FILE;

	return printAppend (buffer, size, 0,
#if defined(DEBUG_HOST_ALLOC)
						"[%p:%p]"
#endif
						"%8p %8lx %8lx %4s %8lx %5d %6d %5d %6u %s\n",
#if defined(DEBUG_HOST_ALLOC)
						pv, pv->psMemArea,
#endif
						MEM_AREA_ToCpuVAddr(pv->psMemArea),
						pv->ui32MMapOffset,
						pv->psMemArea->ui32ByteSize,
						MEM_AREA_TYPE_STRING(pv->psMemArea),
						MEM_AREA_ToCpuPAddr(pv->psMemArea, 0).uiAddr,
						(pv->ui32MemFlags & PVRSRV_HAP_CACHED)? IMG_TRUE: IMG_FALSE,
						(pv->ui32MemFlags & PVRSRV_HAP_WRITECOMBINE)? IMG_TRUE: IMG_FALSE,
						pv->pid,
						pv->ui16Mapped,
						pv->pszName
					   );

}


#ifndef pgprot_noncached
static inline pgprot_t pgprot_noncached(pgprot_t _prot)
{
	unsigned long prot = pgprot_val(_prot);

	#if defined(__arm__)
		prot &= ~(L_PTE_CACHEABLE | L_PTE_BUFFERABLE);
	#else
			#error "mmap.c - pgprot_noncached - Unknown architecture."
	#endif
		
	return __pgprot(prot);
}
#endif


#ifndef pgprot_writecombine
static inline pgprot_t pgprot_writecombine(pgprot_t _prot)
{
	unsigned long prot = pgprot_val(_prot);

	#if defined(__arm__)
		prot &= ~L_PTE_CACHEABLE;
	#else
			#error "mmap.c - pgprot_writecombine - Unknown architecture."
	#endif
	return __pgprot(prot);
}
#endif


static IMG_UINT32 MapIORangeToVMA(struct vm_area_struct *psVma, unsigned long ulFromCpuVAddr,
								  unsigned long ulCpuPAddr, unsigned long ulBytes)
{
#if 0
#endif

    return IMG_IO_REMAP_PAGE_RANGE(psVma, ulFromCpuVAddr, ulCpuPAddr, ulBytes, psVma->vm_page_prot);
}

static IMG_UINT32 MapPageToVMA(struct vm_area_struct *psVma, unsigned long ulFromCpuVAddr, struct page *pPage)
{
#if 0
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
    return vm_insert_page(psVma, ulFromCpuVAddr, pPage);
#else 
    return IMG_REMAP_PAGE_RANGE(psVma, ulFromCpuVAddr, pPage, PAGE_SIZE, psVma->vm_page_prot);
#endif
}

static unsigned PVRMapBlock(struct vm_area_struct *ps_vma,
							PMEM_AREA psMemArea, IMG_UINT32 ui32Offset,
							IMG_UINT32 ui32ByteSize)
{
	IMG_UINT ulAddr;
	IMG_UINT32 ui32Status;

	switch(psMemArea->eAreaType)
	{
		case PVR_MEM_AREA_IOREMAP:
		case PVR_MEM_AREA_IO:
		{
			
			ps_vma->vm_flags |= VM_IO;
			
			ulAddr = MEM_AREA_ToCpuPAddr(psMemArea, ui32Offset).uiAddr;

			
			ulAddr &= ~(PAGE_SIZE-1);

			ui32Status = MapIORangeToVMA(ps_vma, ps_vma->vm_start, ulAddr, ui32ByteSize);
			if(ui32Status != 0)
			{
				PVR_DPF((PVR_DBG_ERROR, "%s: Error - Failed to map memory.\n", __FUNCTION__));
				return IMG_FALSE;
			}
			break;
		}
		case PVR_MEM_AREA_VMALLOC:
		{
			IMG_CHAR *pAddr, *pCurrentAddr;
			unsigned long ulVMAPos;

			
			pAddr=psMemArea->uData.sVmalloc.pvVmallocAddress + ui32Offset;
			pAddr = (IMG_CHAR *)((unsigned long)pAddr & ~(PAGE_SIZE-1));
			pCurrentAddr = pAddr;
			
			ulVMAPos=ps_vma->vm_start;

			while(pCurrentAddr < (pAddr + ui32ByteSize))
			{
				struct page *current_page;

				current_page = vmalloc_to_page(pCurrentAddr);
				ui32Status = MapPageToVMA(ps_vma, ulVMAPos, current_page);
				if(ui32Status != 0)
				{
					PVR_DPF((PVR_DBG_ERROR,"%s: Error - Failed to map memory.\n", __FUNCTION__));
					return IMG_FALSE;
				}
				pCurrentAddr += PAGE_SIZE;
				ulVMAPos += PAGE_SIZE;
			}
			break;
		}
		case PVR_MEM_AREA_ALLOC_PAGES:
		{
			struct page **pvPageList;
			IMG_UINT32 ui32PageIndex, ui32PageCount, i;
			unsigned long ulVMAPos;

			pvPageList = psMemArea->uData.sPageList.pvPageList;
			ui32PageIndex = ui32Offset>>PAGE_SHIFT;
			ui32PageCount = ui32ByteSize>>PAGE_SHIFT;

			
			ulVMAPos=ps_vma->vm_start;
			
			
			for(i=ui32PageIndex; i<(ui32PageIndex+ui32PageCount); i++)
			{
				ui32Status = MapPageToVMA(ps_vma, ulVMAPos, pvPageList[i]);
				if(ui32Status != 0)
				{
					PVR_DPF((PVR_DBG_ERROR,"%s: Error - Failed to map memory.\n", __FUNCTION__));
					return IMG_FALSE;
				}
				ulVMAPos += PAGE_SIZE;
			}
			break;
		}
		default:
		{
			PVR_DPF((PVR_DBG_ERROR, "%s, Invalide mapping flag\n", __FUNCTION__));
			return IMG_FALSE;
		}
	}
	return IMG_TRUE;
}

int PVRMMap(struct file* pFile, struct vm_area_struct* ps_vma)
{
	unsigned long ui32Bytes;
	IMG_UINT32 ui32Offset;
	PKV_OFFSET_STRUCT psCurrentRec;
	IMG_UINT32 ui32Result=0;

	PVR_UNREFERENCED_PARAMETER (pFile);

	ui32Offset = (IMG_UINT32)(ps_vma->vm_pgoff << PAGE_SHIFT);
    LinuxLockMutex(&gPVRSRVLock);
	
	psCurrentRec = FindOffsetStructureByOffset(ui32Offset);
	
	if (!psCurrentRec)
	{
		PVR_DPF((PVR_DBG_ERROR,"pvr_mmap: Error - Attempted to mmap unregistered area at offset 0x%08x\n.\n", ui32Offset));
		LinuxUnLockMutex(&gPVRSRVLock);
		return -ENXIO;
	}

	
	if(psCurrentRec->ui16Mapped == 0)
		psCurrentRec->pid = GetProcessID();

	ui32Bytes = ps_vma->vm_end - ps_vma->vm_start;

	PVR_DPF((PVR_DBG_MESSAGE, "%s: Recieved mmap(2) request with a ui32MMapOffset=0x%08lx,"
                              " and ui32ByteSize=%ld(0x%08lx)\n",
            __FUNCTION__, (ps_vma->vm_pgoff<<PAGE_SHIFT), ui32Bytes, ui32Bytes));

	
	if((ps_vma->vm_flags & VM_WRITE) && !(ps_vma->vm_flags & VM_SHARED))
	{
		PVR_DPF((PVR_DBG_ERROR,"pvr_mmap : Error - Cannot mmap non-shareable writable areas.\n"));
	    LinuxUnLockMutex(&gPVRSRVLock);
		return -EINVAL;
	}

	
	
	ps_vma->vm_flags |= VM_RESERVED;

	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15))
	ps_vma->vm_flags |= VM_DONTEXPAND;
#endif
	
	ps_vma->vm_private_data = (void *)psCurrentRec;

	if(!(psCurrentRec->ui32MemFlags & PVRSRV_HAP_CACHED))
	{
		if(psCurrentRec->ui32MemFlags & PVRSRV_HAP_WRITECOMBINE)
		{
			ps_vma->vm_page_prot = pgprot_writecombine(ps_vma->vm_page_prot);
		}
		else
		{
			ps_vma->vm_page_prot = pgprot_noncached(ps_vma->vm_page_prot);
		}
	}

	if(psCurrentRec->psMemArea)
		ui32Result = PVRMapBlock(ps_vma, psCurrentRec->psMemArea, 0, ui32Bytes);
	else
		printk("PVRMapBlock: psCurrentRec->psMemArea is null\n");

	if(ui32Result == 0)
	{
		PVR_DPF((PVR_DBG_ERROR,"pvr_mmap: Error - Failed to map contiguous pages.\n"));
		LinuxUnLockMutex(&gPVRSRVLock);
		return -EAGAIN;
	}
	
	
	ps_vma->vm_ops = &PVRMMapVMOps;

	
	PVRMMapVOpen(ps_vma);

	PVR_DPF((PVR_DBG_MESSAGE,"pvr_mmap: Mapped area at offset 0x%08x\n", ui32Offset));

    LinuxUnLockMutex(&gPVRSRVLock);

	return 0;
}


static void PVRMMapVOpen(struct vm_area_struct* ps_vma)
{
	PKV_OFFSET_STRUCT psRec = (PKV_OFFSET_STRUCT)ps_vma->vm_private_data;

	PVR_ASSERT(psRec != IMG_NULL)

	psRec->ui16Mapped++;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	MOD_INC_USE_COUNT;
#endif
}


static void PVRMMapVClose(struct vm_area_struct* ps_vma)
{
	IMG_VOID			*pvVirtAddress;
	PKV_OFFSET_STRUCT psRec = (PKV_OFFSET_STRUCT)ps_vma->vm_private_data;

	PVR_ASSERT(psRec != IMG_NULL)

	psRec->ui16Mapped--;

	pvVirtAddress = MEM_AREA_ToCpuVAddr(psRec->psMemArea);

	PVR_DPF((PVR_DBG_MESSAGE, "PVRMMapVClose: area %p: mapped=%d", pvVirtAddress, psRec->ui16Mapped));

	
	if(psRec->ui16Mapped == 0)
	{
		PVRMMapRemoveRegisteredArea(pvVirtAddress);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	MOD_DEC_USE_COUNT;
#endif
}

PKV_OFFSET_STRUCT *PVRMMapRegisterArea(const char * name, PMEM_AREA psMemArea, IMG_UINT32 ui32Flags)
{
	PKV_OFFSET_STRUCT psNextAllocRec;

	PVR_DPF((PVR_DBG_MESSAGE,
			"PVRMMapregister_area(%s, %08lX, %08lX)",
			name, MEM_AREA_ToCpuVAddr(psMemArea), ui32Flags));

	if( (psNextAllocRec = FindOffsetStructBy_MEM_AREA(psMemArea))!= 0)
	{
		PVR_DPF((PVR_DBG_ERROR,"Memory at %p is already registered", MEM_AREA_ToCpuVAddr(psMemArea)));
		return psNextAllocRec;
	}

	ui32RegisteredAreas++;
	if(ui32RegisteredAreas == MAX_MMAP_AREAS)
	{
		PVR_DPF((PVR_DBG_ERROR,"Cannot add another mapping without exceeed table size %d", MAX_MMAP_AREAS));
		return NULL;
	}
	psNextAllocRec = KMemCacheAllocWrapper(psMemmapCache, GFP_KERNEL);

	if (!psNextAllocRec)
	{
		PVR_DPF((PVR_DBG_ERROR,"Couldn't alloc another mapping record from cache"));
		return NULL;
	}
	psNextAllocRec->ui32MMapOffset	= GetFirstFreePageAlignedNumber();
	psNextAllocRec->psMemArea		= psMemArea;
	psNextAllocRec->pszName			= name;
	psNextAllocRec->ui32MemFlags	= ui32Flags;
	psNextAllocRec->pid				= GetProcessID();
	psNextAllocRec->ui16Mapped		= 0;
	psNextAllocRec->psNext			= psKVOffsetTable;
	psKVOffsetTable = psNextAllocRec;
	return psNextAllocRec;
}


PVRSRV_ERROR
PVRMMapVerifyRegisteredAddress(IMG_VOID *pvMem)
{
	PKV_OFFSET_STRUCT pEntry;
	pEntry = FindOffsetStructureByKVAddr(pvMem);
	return pEntry ? PVRSRV_OK : PVRSRV_ERROR_BAD_MAPPING;
}


PVRSRV_ERROR PVRMMapGetFullMapData(IMG_VOID *pvKVAddr,
                                        IMG_UINT32 ui32Bytes,
                                        IMG_PVOID *ppvKVBase,
                                        IMG_UINT32 *pui32BaseBytes,
                                        IMG_UINT32 *pui32Offset,
                                        PVR_MMAP_TYPE *peMapType)
{
	PKV_OFFSET_STRUCT pEntry;

	if(!ui32Bytes)
	{
		ui32Bytes = PAGE_SIZE;
	}
	
	*pui32BaseBytes = ui32Bytes;
	pEntry = FindOffsetStructureByKVAddr(pvKVAddr);
	if (!pEntry)
	{
		return PVRSRV_ERROR_BAD_MAPPING;
	}

	*ppvKVBase = MEM_AREA_ToCpuVAddr(pEntry->psMemArea);
	*pui32BaseBytes = pEntry->psMemArea->ui32ByteSize;
	
	switch(pEntry->psMemArea->eAreaType)
	{
		case PVR_MEM_AREA_IOREMAP:
		case PVR_MEM_AREA_IO:
			*peMapType = PVR_MMAP_CONTIG;
			break;
		case PVR_MEM_AREA_VMALLOC:
		case PVR_MEM_AREA_ALLOC_PAGES:
			*peMapType = PVR_MMAP_VIRTUAL;
			break;
		default:
			PVR_DPF((PVR_DBG_ERROR, "PVRMMapGetFullMapData: Invalide mapping flags\n"));
			break;
	}

	*pui32Offset = pEntry->ui32MMapOffset;
	return PVRSRV_OK;
}


PVRSRV_ERROR
PVRMMapRemoveRegisteredArea(IMG_VOID *pvVirtAddress)
{
	PKV_OFFSET_STRUCT *ppv, pv;
    IMG_UINT8 *pui8CpuVAddr = NULL;
    IMG_UINT8 *pui8IndexCpuVAddr = (IMG_UINT8 *)pvVirtAddress;
	
	for (ppv=&psKVOffsetTable; (pv = *ppv); ppv=&(*ppv)->psNext)
	{
		PMEM_AREA psMemArea = pv->psMemArea;

		pui8CpuVAddr = MEM_AREA_ToCpuVAddr(psMemArea);

		
		if(pui8CpuVAddr)
		{
			if(pui8IndexCpuVAddr >= pui8CpuVAddr
				&& (pui8IndexCpuVAddr <= (pui8CpuVAddr + psMemArea->ui32ByteSize)))
			{
				break;
			}
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR, "%s: failed to find virtual address for allocation.\n", __FUNCTION__));
			return PVRSRV_ERROR_BAD_MAPPING;
		}
    }

	if (!pv)
	{
		PVR_DPF((PVR_DBG_ERROR, "Failed to find offset struct (KVAddress=%p)\n", pvVirtAddress));
		return PVRSRV_ERROR_BAD_MAPPING;
	}

	if(pv->ui16Mapped)
	{
		PVR_DPF((PVR_DBG_MESSAGE, "%s: Not unregistering still-mapped area at 0x%p\n", __FUNCTION__, pvVirtAddress));
		


		return PVRSRV_OK;
	}

	ui32RegisteredAreas--;

	if(pv->psNext)
		(*ppv)->ui32MMapOffset =
				pv->psNext->ui32MMapOffset + ((pv->psNext->psMemArea->ui32ByteSize + PAGE_SIZE - 1) & HOST_PAGEMASK);
	else
		(*ppv)->ui32MMapOffset = PAGE_SIZE;
	(*ppv)= pv->psNext;

	
	MEM_AREA_AreaDeepFree(pv->psMemArea);

	KMemCacheFreeWrapper(psMemmapCache, pv);
	
	return PVRSRV_OK;
}


static PKV_OFFSET_STRUCT FindOffsetStructureByOffset(IMG_UINT32 ui32Offset)
{
	PKV_OFFSET_STRUCT pv;

	for (pv = psKVOffsetTable; pv; pv = pv->psNext)
	{
		if (pv->ui32MMapOffset == ui32Offset)
		{
			break;
		}
	}
	return pv;
}


static IMG_UINT32 GetFirstFreePageAlignedNumber(void)
{
    if(!psKVOffsetTable)
    {
        return PAGE_SIZE;
    }

	return psKVOffsetTable->ui32MMapOffset + ((psKVOffsetTable->psMemArea->ui32ByteSize + PAGE_SIZE - 1) & HOST_PAGEMASK);
}

static PKV_OFFSET_STRUCT FindOffsetStructureByKVAddr(IMG_VOID *pvVirtAddress)
{
    PKV_OFFSET_STRUCT psOffsetStruct;
    IMG_UINT8 *pui8CpuVAddr = NULL;
    IMG_UINT8 *pui8IndexCpuVAddr = (IMG_UINT8 *)pvVirtAddress;


    for(psOffsetStruct=psKVOffsetTable; psOffsetStruct; psOffsetStruct=psOffsetStruct->psNext)
    {
        PMEM_AREA psMemArea = psOffsetStruct->psMemArea;

		pui8CpuVAddr = MEM_AREA_ToCpuVAddr(psMemArea);
        
		
        if(pui8CpuVAddr)
        {
            if(pui8IndexCpuVAddr >= pui8CpuVAddr
               && (pui8IndexCpuVAddr) <= (pui8CpuVAddr + psMemArea->ui32ByteSize))
            {
                return psOffsetStruct;
            }
            else
            {
                pui8CpuVAddr = NULL;
            }
        }
    }
    printk(KERN_ERR "%s: Failed to find allocation struct (KVAddress=%p)\n", __FUNCTION__, pvVirtAddress);
    return NULL;
}

static PKV_OFFSET_STRUCT FindOffsetStructBy_MEM_AREA(PMEM_AREA psLinuxMemArea)
{
    PKV_OFFSET_STRUCT psOffsetStruct = NULL;
    for(psOffsetStruct=psKVOffsetTable; psOffsetStruct; psOffsetStruct=psOffsetStruct->psNext)
    {
        if(psOffsetStruct->psMemArea == psLinuxMemArea)
        {
            return psOffsetStruct;
        }
    }
    return NULL;
}

void PVRMMapInit(void)
{
	if(LinuxMMInit() != PVRSRV_OK)
	{
		PVR_DPF((PVR_DBG_ERROR,"PVRMMapinit: failed to allocate map-area cache"));
	}

	psKVOffsetTable = 0;
	psMemmapCache = KMemCacheCreateWrapper("img-mmap", sizeof(KV_OFFSET_STRUCT), 0, 0);

	if (psMemmapCache)
	{
		CreateProcReadEntry("mmap", PrintVirtualMemRegistrations);
	}
	else
	{
		PVR_DPF((PVR_DBG_ERROR,"PVRMMapinit: failed to allocate kmem_cache"));
	}
}


void PVRMMapCleanup(void)
{
	IMG_VOID *pvVirtAddress;
	if (!psMemmapCache)
		return;

	if (ui32RegisteredAreas > 0)
	{
		while (psKVOffsetTable)
		{
			pvVirtAddress = MEM_AREA_ToCpuVAddr(psKVOffsetTable->psMemArea);
			psKVOffsetTable->pid = 0;
			PVR_DPF((PVR_DBG_WARNING, "PVRMMapcleanup: belatedly unregistering %p", pvVirtAddress));
			PVRMMapRemoveRegisteredArea(pvVirtAddress);
		}
	}

	
	RemoveProcEntry("mmap");

	if(psMemmapCache)
		KMemCacheDestroyWrapper(psMemmapCache);

	LinuxMMCleanup();

	psMemmapCache = 0;
	PVR_DPF((PVR_DBG_MESSAGE,"PVRMMapcleanup: KVOffsetTable deallocated"));
}

