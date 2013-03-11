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

#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
#include <config/modversions.h>
#define MODVERSIONS
#endif

#include <linux/version.h>
#include <asm/page.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>
#include <asm/io.h>

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>

#include "services_headers.h"

#include "mm.h"

#if defined(DEBUG_HOST_ALLOC)
#include "proc.h"
#endif


static LinuxKMemCache	*psMemAreaCache = 0;

static MEM_AREA *LinuxMemAreaStructAlloc(IMG_VOID);

static IMG_VOID MEM_AREA_FreeVMalloc(MEM_AREA *psLinuxMemArea);
static IMG_VOID MEM_AREA_FreeAllocPages(MEM_AREA *psLinuxMemArea);
static IMG_VOID MEM_AREA_FreeIO(MEM_AREA *psLinuxMemArea);
static IMG_VOID MEM_AREA_FreeIORemap(MEM_AREA *psLinuxMemArea);

#if defined(DEBUG_HOST_ALLOC)
typedef struct _DEBUG_MEM_ALLOC_
{
	void			*pvAddr;
	void			*psMemArea;
	IMG_UINT32		ui32ByteSize;
	char			*psFileName;
	char			*type;
	IMG_UINT32		ui32Line;
	struct _DEBUG_MEM_ALLOC_	*psNextMemAlloc;
}DEBUG_MEM_ALLOC, *PDEBUG_MEM_ALLOC;

PDEBUG_MEM_ALLOC gpsMemDebug = NULL;

#define DebugAddMemAlloc(addr, size, type, memarea) _DebugAddMemAlloc(addr, size, type, memarea, __FUNCTION__, __LINE__)
void _DebugAddMemAlloc(void *pvAddr, IMG_UINT32 ui32ByteSize, char * type, PMEM_AREA psMemArea,
					   char *fileName, int line)
{
	PDEBUG_MEM_ALLOC psMemDebug;
	psMemDebug = kmalloc(sizeof(DEBUG_MEM_ALLOC), GFP_KERNEL);
	psMemDebug->pvAddr = pvAddr;
	psMemDebug->psMemArea = psMemArea;
	psMemDebug->ui32ByteSize = ui32ByteSize;
	psMemDebug->psFileName = fileName;
	psMemDebug->ui32Line = line;
	psMemDebug->type = type;
	psMemDebug->psNextMemAlloc = NULL;
	
	if(gpsMemDebug)
	{
		psMemDebug->psNextMemAlloc = gpsMemDebug;
	}
	gpsMemDebug = psMemDebug;
}

void DebugRemoveMemAlloc(void *pvAddr)
{
	PDEBUG_MEM_ALLOC *ppsMemDbg, psMemDbg;
	ppsMemDbg = &gpsMemDebug;
	while(*ppsMemDbg)
	{
		psMemDbg = *ppsMemDbg;
		if(psMemDbg->pvAddr == pvAddr)
		{
#if 0
			
 			psMemDbg->type = "free";
#else
			*ppsMemDbg = psMemDbg->psNextMemAlloc;
			kfree(psMemDbg);
#endif
			return;
		}
		ppsMemDbg = &((*ppsMemDbg)->psNextMemAlloc);
	}
}

static off_t HostMemRegistrations(char * buffer, size_t size, off_t off)
{
	PDEBUG_MEM_ALLOC psMemDebug;
	off_t Ret;
	
	if(!off)
	{
		Ret = printAppend(buffer, size, 0, "Address    Size    Type\n");
	}
	else
	{
		for(psMemDebug=gpsMemDebug;
			psMemDebug && --off;
			psMemDebug = psMemDebug->psNextMemAlloc);
		
		if(psMemDebug)
		{
			Ret =  printAppend(buffer, size, 0,
							   "%p %p %p %4d %4s %-20s\n",
				psMemDebug->psMemArea, psMemDebug->pvAddr, psMemDebug->ui32ByteSize, psMemDebug->ui32Line,
				MEM_AREA_TYPE_STRING(psMemDebug->psMemArea), psMemDebug->psFileName);
		}
		else
		{
			Ret = END_OF_FILE;
		}
	}
	return Ret;
}
#endif

PVRSRV_ERROR LinuxMMInit(IMG_VOID)
{
	psMemAreaCache = KMemCacheCreateWrapper("img-mm", sizeof(MEM_AREA), 0, 0);
	if(!psMemAreaCache)
	{
		PVR_DPF((PVR_DBG_ERROR,"%s: failed to allocate kmem_cache", __FUNCTION__));
		return PVRSRV_ERROR_OUT_OF_MEMORY;
	}
#if defined(DEBUG_HOST_ALLOC)
	CreateProcReadEntry("img-mm", HostMemRegistrations);
#endif
	return PVRSRV_OK;
}

IMG_VOID LinuxMMCleanup(IMG_VOID)
{
	if(psMemAreaCache)
	{
#if defined(DEBUG_HOST_ALLOC)
		PDEBUG_MEM_ALLOC psMemDbg, psMemDbgOld;
		if(gpsMemDebug)
		{
			PVR_DPF((PVR_DBG_ERROR, "s: Not all memory areas are freed, attempting to free it now\n", __FUNCTION__));
			
			psMemDbg = gpsMemDebug;
			while(psMemDbg)
			{
				PVR_DPF((PVR_DBG_ERROR, "Freeing allocation at %08X\n", psMemDbg));
				psMemDbgOld = psMemDbg;
				psMemDbg = psMemDbg->psNextMemAlloc;
				LinuxMemAreaStructFree(psMemDbgOld->psMemArea);
				kfree(psMemDbgOld);
			}
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR, "gpsMemDebug: found NULL!\n"));
		}
#endif 

		KMemCacheDestroyWrapper(psMemAreaCache); 
		psMemAreaCache=NULL;
	}
}

static MEM_AREA * LinuxMemAreaStructAlloc(IMG_VOID)
{
	return KMemCacheAllocWrapper(psMemAreaCache, GFP_KERNEL);
}

IMG_VOID LinuxMemAreaStructFree(MEM_AREA *psLinuxMemArea)
{
	PVR_ASSERT(psLinuxMemArea);
	
	KMemCacheFreeWrapper(psMemAreaCache, psLinuxMemArea);
}

PMEM_AREA MEM_AREA_VMalloc(IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags)
{
	MEM_AREA *psLinuxMemArea;
	IMG_VOID *pvCpuVAddr;
	
	psLinuxMemArea = LinuxMemAreaStructAlloc();
	if(!psLinuxMemArea)
	{
		goto failed;
	}
	
	pvCpuVAddr = VMallocWrapper(ui32Bytes, ui32MappingFlags);
	if(!pvCpuVAddr)
	{
		goto failed;
	}
	
	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
	
	ReservePages(pvCpuVAddr, ui32Bytes);
#endif
	
	psLinuxMemArea->eAreaType = PVR_MEM_AREA_VMALLOC;
	psLinuxMemArea->uData.sVmalloc.pvVmallocAddress = pvCpuVAddr;
	psLinuxMemArea->ui32ByteSize = ui32Bytes;
	
#if defined(DEBUG_HOST_ALLOC)
	DebugAddMemAlloc(pvCpuVAddr, ui32Bytes, "vmalloc", psLinuxMemArea);
#endif
	
	return psLinuxMemArea;
	
failed:
		PVR_DPF((PVR_DBG_ERROR, "%s: failed!"));
	if(psLinuxMemArea)
		LinuxMemAreaStructFree(psLinuxMemArea);
	return NULL;
}

PMEM_AREA MEM_AREA_NewAllocPages(IMG_UINT32 ui32Bytes)
{
	PMEM_AREA psLinuxMemArea;
	IMG_UINT32 ui32PageCount;
	struct page **pvPageList;
	IMG_UINT32 i;
	
	psLinuxMemArea = LinuxMemAreaStructAlloc();
	if(!psLinuxMemArea)
	{
		goto failed_area_alloc;
	}
	
	ui32PageCount = PAGE_ALIGN(ui32Bytes)>>PAGE_SHIFT;
	
	pvPageList = VMallocWrapper(sizeof(void *) * ui32PageCount, PVRSRV_HAP_CACHED);
	if(!pvPageList)
	{
		goto failed_vmalloc;
	}
	
	for(i=0; i<ui32PageCount; i++)
	{
		pvPageList[i] = alloc_pages(GFP_KERNEL, 0);
		if(!pvPageList[i])
		{
			goto failed_alloc_pages;
		}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
		
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))		
		SetPageReserved(pvPageList[i]);
#else
		mem_map_reserve(pvPageList[i]);
#endif
#endif
#if defined(DEBUG_HOST_ALLOC)
		DebugAddMemAlloc(pvPageList[i], PAGE_SIZE, "alloc_pages", psLinuxMemArea);
#endif

	}
	psLinuxMemArea->eAreaType = PVR_MEM_AREA_ALLOC_PAGES;
	psLinuxMemArea->uData.sPageList.pvPageList = pvPageList;
	psLinuxMemArea->ui32ByteSize = ui32PageCount * PAGE_SIZE;
	
	return psLinuxMemArea;

failed_alloc_pages:
	for(i--;i>=0;i--)
	{
		__free_pages(pvPageList[i], 0);
	}
	VFreeWrapper(pvPageList);
	failed_vmalloc:
	LinuxMemAreaStructFree(psLinuxMemArea);
	failed_area_alloc:
	PVR_DPF((PVR_DBG_ERROR, "%s: failed", __FUNCTION__));
	
	return NULL;
}

MEM_AREA * MEM_AREA_NewIORemap(IMG_CPU_PHYADDR BasePAddr, 
							   IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags)
{
	MEM_AREA *psLinuxMemArea;
	IMG_VOID *pvIORemapCookie;
	
	psLinuxMemArea = LinuxMemAreaStructAlloc();
	if(!psLinuxMemArea)
	{
		return NULL;
	}
	
	pvIORemapCookie = IORemapWrapper(BasePAddr, ui32Bytes, ui32MappingFlags);
	if(!pvIORemapCookie)
	{
		LinuxMemAreaStructFree(psLinuxMemArea);
		return NULL;
	}
	
	psLinuxMemArea->eAreaType = PVR_MEM_AREA_IOREMAP;
	psLinuxMemArea->uData.sIORemap.pvIORemapCookie = pvIORemapCookie;
	psLinuxMemArea->uData.sIORemap.CPUPhysAddr = BasePAddr;
	psLinuxMemArea->ui32ByteSize = ui32Bytes;
	
#if defined(DEBUG_HOST_ALLOC)
	DebugAddMemAlloc(pvIORemapCookie, ui32Bytes, "ioremap", psLinuxMemArea);
#endif
	
	return psLinuxMemArea;
}

static IMG_VOID MEM_AREA_FreeVMalloc(MEM_AREA *psLinuxMemArea)
{
	PVR_ASSERT(psLinuxMemArea);
	PVR_ASSERT(psLinuxMemArea->eAreaType == PVR_MEM_AREA_VMALLOC);
	PVR_ASSERT(psLinuxMemArea->uData.sVmalloc.pvVmallocAddress);
	
	
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
	UnreservePages(psLinuxMemArea->uData.sVmalloc.pvVmallocAddress,
				   psLinuxMemArea->ui32ByteSize);
#endif
	
	VFreeWrapper(psLinuxMemArea->uData.sVmalloc.pvVmallocAddress);

#if defined(DEBUG_HOST_ALLOC)
	
	DebugRemoveMemAlloc(psLinuxMemArea->uData.sVmalloc.pvVmallocAddress);
#endif
	
	LinuxMemAreaStructFree(psLinuxMemArea);
	return;
}

static IMG_VOID MEM_AREA_FreeAllocPages(MEM_AREA *psLinuxMemArea)
{
	IMG_UINT32 ui32PageCount;
	struct page **pvPageList;
	IMG_UINT32 i;
	
	PVR_ASSERT(psLinuxMemArea);
	PVR_ASSERT(psLinuxMemArea->eAreaType == PVR_MEM_AREA_ALLOC_PAGES);
	
	ui32PageCount = PAGE_ALIGN(psLinuxMemArea->ui32ByteSize)>>PAGE_SHIFT;
	pvPageList = psLinuxMemArea->uData.sPageList.pvPageList;
	
	for(i=0;i<ui32PageCount;i++)
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))		
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))		
		ClearPageReserved(pvPageList[i]);
#else
		mem_map_reserve(pvPageList[i]);
#endif		
#endif	
#if defined(DEBUG_HOST_ALLOC)
		DebugRemoveMemAlloc(pvPageList[i]);
#endif
		__free_pages(pvPageList[i], 0);
	}
	VFreeWrapper(psLinuxMemArea->uData.sPageList.pvPageList);
	LinuxMemAreaStructFree(psLinuxMemArea);
}

static IMG_VOID MEM_AREA_FreeIO(MEM_AREA *psLinuxMemArea)
{
	PVR_ASSERT(psLinuxMemArea);
	PVR_ASSERT(psLinuxMemArea->eAreaType == PVR_MEM_AREA_IO);
	
#if defined(DEBUG_HOST_ALLOC)
			DebugRemoveMemAlloc(MEM_AREA_ToCpuVAddr(psLinuxMemArea));
#endif
	
	LinuxMemAreaStructFree(psLinuxMemArea);
	return;
}

static IMG_VOID MEM_AREA_FreeIORemap(MEM_AREA *psLinuxMemArea)
{
	PVR_ASSERT(psLinuxMemArea);
	PVR_ASSERT(psLinuxMemArea->eAreaType == PVR_MEM_AREA_IOREMAP);
	
#if defined(DEBUG_HOST_ALLOC)
			DebugRemoveMemAlloc(MEM_AREA_ToCpuVAddr(psLinuxMemArea));
#endif
	IOUnmapWrapper(psLinuxMemArea->uData.sIORemap.pvIORemapCookie);
	LinuxMemAreaStructFree(psLinuxMemArea);
	return;
}

IMG_VOID MEM_AREA_AreaDeepFree(MEM_AREA *psLinuxMemArea)
{
	switch(psLinuxMemArea->eAreaType)
	{
		case PVR_MEM_AREA_VMALLOC:
			MEM_AREA_FreeVMalloc(psLinuxMemArea);
			break;
		case PVR_MEM_AREA_ALLOC_PAGES:
			MEM_AREA_FreeAllocPages(psLinuxMemArea);
			break;
		case PVR_MEM_AREA_IOREMAP:
			MEM_AREA_FreeIORemap(psLinuxMemArea);
			break;
		case PVR_MEM_AREA_IO:
			MEM_AREA_FreeIO(psLinuxMemArea);
			break;
		default:
			PVR_DPF((PVR_DBG_ERROR, "%s: Unknown are type (%d)\n",
					 __FUNCTION__, psLinuxMemArea->eAreaType));
	}
}

IMG_VOID * MEM_AREA_ToCpuVAddr(MEM_AREA *psLinuxMemArea)
{
	PVR_ASSERT(psLinuxMemArea);

	switch(psLinuxMemArea->eAreaType)
	{
		case PVR_MEM_AREA_ALLOC_PAGES:
			return __va(page_to_phys(psLinuxMemArea->uData.sPageList.pvPageList[0]));
		
		case PVR_MEM_AREA_VMALLOC:
			return psLinuxMemArea->uData.sVmalloc.pvVmallocAddress;
	
		case PVR_MEM_AREA_IOREMAP:
		case PVR_MEM_AREA_IO:
			return psLinuxMemArea->uData.sIORemap.pvIORemapCookie;
	
		default:
			return NULL;
	}
}

IMG_CPU_PHYADDR MEM_AREA_ToCpuPAddr(PMEM_AREA psMemArea, IMG_UINT32 ui32ByteOffset)
{
	IMG_CPU_PHYADDR CpuPAddr;

	PVR_ASSERT(psMemArea);

	CpuPAddr.uiAddr = 0;

	switch(psMemArea->eAreaType)
	{
		case PVR_MEM_AREA_IOREMAP:
		{
			CpuPAddr = psMemArea->uData.sIORemap.CPUPhysAddr;
			CpuPAddr.uiAddr += ui32ByteOffset;
			break;
		}
		case PVR_MEM_AREA_IO:
		{
			CpuPAddr = psMemArea->uData.sIO.CPUPhysAddr;
			CpuPAddr.uiAddr += ui32ByteOffset;
			break;
		}
		case PVR_MEM_AREA_VMALLOC:
		{
			IMG_CHAR *pCpuVAddr;
			struct page *page;
			pCpuVAddr = (IMG_CHAR *)psMemArea->uData.sVmalloc.pvVmallocAddress;
			pCpuVAddr += ui32ByteOffset;
			page = ConvertKVToPage(pCpuVAddr);
			CpuPAddr.uiAddr = page_to_phys(page);
			CpuPAddr.uiAddr += ui32ByteOffset & (PAGE_SIZE - 1);
			break;
		}
		case PVR_MEM_AREA_ALLOC_PAGES:
		{
			struct page *page;
			IMG_UINT32 ui32PageIndex = ui32ByteOffset >> PAGE_SHIFT;
			page = psMemArea->uData.sPageList.pvPageList[ui32PageIndex];
			CpuPAddr.uiAddr = page_to_phys(page);
			CpuPAddr.uiAddr += ui32ByteOffset & (PAGE_SIZE - 1);
			break;
		}
		default:
			PVR_DPF((PVR_DBG_ERROR, "%s: Unknown MEM_AREA type (%d)\n",
					 __FUNCTION__, psMemArea->eAreaType));
	}

	PVR_ASSERT(CpuPAddr.uiAddr);
	return CpuPAddr;
}

IMG_VOID KMemCacheDestroyWrapper(LinuxKMemCache *psCache)
{
	kmem_cache_destroy(psCache);
}

IMG_VOID IOUnmapWrapper(IMG_VOID *pvIORemapCookie)
{
	iounmap(pvIORemapCookie);
}

IMG_VOID *_IORemapWrapper(IMG_CPU_PHYADDR BasePAddr, IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags,
						IMG_CHAR *pszFileName, IMG_UINT32 ui32Line)
{
	IMG_VOID *pvIORemapCookie = IMG_NULL;

	PVR_DPF((PVR_DBG_MESSAGE, "Mapping flags:%8x", ui32MappingFlags));
	
	switch(ui32MappingFlags & PVRSRV_HAP_CACHETYPE_MASK)
	{
		case PVRSRV_HAP_CACHED:
			pvIORemapCookie = (IMG_VOID *)IOREMAP(BasePAddr.uiAddr, ui32Bytes);
			break;
		case PVRSRV_HAP_WRITECOMBINE:
			pvIORemapCookie = (IMG_VOID *)IOREMAP_WC(BasePAddr.uiAddr, ui32Bytes);
			break;
		case PVRSRV_HAP_UNCACHED:
			pvIORemapCookie = (IMG_VOID *)IOREMAP_UC(BasePAddr.uiAddr, ui32Bytes);
			break;
		default:
			PVR_DPF((PVR_DBG_ERROR, "IORemapWrapper: unknown mapping flags"));
			return NULL;
	}
	return pvIORemapCookie;
}

IMG_VOID KMemCacheFreeWrapper(LinuxKMemCache *psCache, IMG_VOID *pvObject)
{
	PVR_ASSERT(psCache);
	PVR_ASSERT(pvObject);
	
	kmem_cache_free(psCache, pvObject);
}

LinuxKMemCache *KMemCacheCreateWrapper(IMG_CHAR *pszName, size_t Size, 
									   size_t Align, IMG_UINT32 ui32Flags)
{
#if defined(DEBUG_LINUX_SLAB_ALLOCATIONS)
	ui32Flags |= SLAB_POISON|SLAB_RED_ZONE;
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23))
	return kmem_cache_create(pszName, Size, Align, ui32Flags, NULL);
#else
	return kmem_cache_create(pszName, Size, Align, ui32Flags, NULL, NULL);
#endif
}

IMG_VOID *_KMemCacheAllocWrapper(LinuxKMemCache *psCache, gfp_t Flags, 
								 IMG_CHAR *pszFileName, IMG_UINT32 ui32Line)
{
	IMG_VOID *pvRet;
	PVR_UNREFERENCED_PARAMETER(pszFileName);
	PVR_UNREFERENCED_PARAMETER(ui32Line);

	PVR_ASSERT(psCache);
	
	pvRet = kmem_cache_alloc(psCache, Flags);
	return pvRet;
}

IMG_VOID VFreeWrapper(IMG_VOID *pvCpuVAddr)
{
#if defined(DEBUG_LINUX_MEMORY_ALLOCATIONS)
	DebugMemAllocRecordRemove(DEBUG_MEM_ALLOC_TYPE_VMALLOC, pvCpuVAddr);
#endif
	vfree(pvCpuVAddr);
}

IMG_VOID *_VMallocWrapper(IMG_UINT32 ui32Bytes, IMG_UINT32 ui32AllocFlags, 
						  IMG_CHAR *pszFileName, IMG_UINT32 ui32Line)
{
	pgprot_t PGProtFlags;
	IMG_VOID *pvRet;
	PVR_UNREFERENCED_PARAMETER(pszFileName);
	PVR_UNREFERENCED_PARAMETER(ui32Line);
	
	if(ui32AllocFlags & PVRSRV_HAP_CACHED)
	{
		PGProtFlags = PAGE_KERNEL;
	}
	else
#if defined(__arm__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	if(ui32AllocFlags & PVRSRV_HAP_WRITECOMBINE)
	{
		PGProtFlags = pgprot_writecombine(PAGE_KERNEL);
	}
	else
#endif
	{
		PGProtFlags = pgprot_noncached(PAGE_KERNEL);
	}
	
		
	pvRet = __vmalloc(ui32Bytes, GFP_KERNEL | __GFP_HIGHMEM, PGProtFlags);
	
#if defined(DEBUG_LINUX_MEMORY_ALLOCATIONS)
	if(pvRet)
	{
		DebugMemAllocRecordAdd(DEBUG_MEM_ALLOC_TYPE_VMALLOC,
				pvRet,
				pvRet,
				0,
				NULL,
				PAGE_ALIGN(ui32Bytes),
				pszFileName,
				ui32Line
				);
	}
#endif
	return pvRet;
}

struct page *ConvertKVToPage(IMG_PVOID pvAddr)
{
	struct page *rv = 0;

#if (LINUX_VERSION_CODE >= 0x020508) || ((LINUX_VERSION_CODE >= 0x020414) && (LINUX_VERSION_CODE < 0x020500))

	rv = vmalloc_to_page(pvAddr);

#else 

	IMG_UINT32 ui32Addr = (IMG_UINT32)pvAddr;
	pgd_t *ppgd;
	pmd_t *ppmd;
	pte_t *ppte, pte;

	{
		
		pvAddr = (IMG_VOID *) VMALLOC_VMADDR(pvAddr);

		
		ppgd = pgd_offset_k(ui32Addr);

		
		if (!pgd_none(*ppgd))
		{
			
			ppmd = pmd_offset(ppgd, ui32Addr);

			
			if (!pmd_none(*ppmd))
			{
				
#ifndef PVR_ATOMIC_PTE
				ppte = pte_offset(ppmd, ui32Addr);
				pte = *ppte;
#else
				ppte = pte_offset_atomic(ppmd, ui32Addr);
				pte = *ppte;
				pte_kunmap(ppte);
#endif
				
				if (pte_present(pte))
				{
					
					rv = pte_page(pte);
				}
				else
				{
					PVR_DPF((PVR_DBG_ERROR,"ConvertKVToPage: Failed to find a valid page table entry"));
				}
			}
			else
			{
				PVR_DPF((PVR_DBG_ERROR,"ConvertKVToPage: Failed to find a valid mid-level page directory"));
			}
		}
		else
		{
			PVR_DPF((PVR_DBG_ERROR,"ConvertKVToPage: Failed to find a valid page directory"));
		}
	}
#endif
	return rv;
}
