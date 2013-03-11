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

#if !defined(_MM_H_)
#define _MM_H_

#include <linux/version.h>

#include <linux/highmem.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
typedef kmem_cache_t LinuxKMemCache;
#else
typedef struct kmem_cache LinuxKMemCache;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15))
typedef int gfp_t;
#endif

#if defined(__i386__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	#define	IOREMAP(pa, bytes)	ioremap_cache(pa, bytes)
#else	
	#if defined(__arm__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		#define	IOREMAP(pa, bytes)	ioremap_cached(pa, bytes)
	#else
		#define IOREMAP(pa, bytes)	ioremap(pa, bytes)
	#endif
#endif

#if defined(__arm__)
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		#define IOREMAP_WC(pa, bytes) ioremap_wc(pa, bytes)
	#else
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
			#define	IOREMAP_WC(pa, bytes)	ioremap_nocache(pa, bytes)
		#else
			#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) || (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17))
				#define	IOREMAP_WC(pa, bytes)	__ioremap(pa, bytes, L_PTE_BUFFERABLE)
			#else
				#define IOREMAP_WC(pa, bytes)	__ioremap(pa, bytes, L_PTE_BUFFERABLE, 1)
			#endif
		#endif
	#endif
#else
	#define IOREMAP_WC(pa, bytes)	ioremap_nocache(pa, bytes)
#endif

#define	IOREMAP_UC(pa, bytes)	ioremap_nocache(pa, bytes)

#if !defined(mem_map_reserve)
#define mem_map_reserve(p)	set_bit(PG_reserved, &((p)->flags))
#define mem_map_unreserve(p)	clear_bit(PG_reserved, &((p)->flags))
#endif

typedef enum {
    PVR_MEM_AREA_IOREMAP = 0,
    PVR_MEM_AREA_IO,
    PVR_MEM_AREA_VMALLOC,
    PVR_MEM_AREA_ALLOC_PAGES,
    PVR_MEM_AREA_TYPE_COUNT
}PVR_MEM_AREA_TYPE;


#define MEM_AREA_TYPE_STRING(mem) \
	((((PMEM_AREA)mem)->eAreaType == PVR_MEM_AREA_IOREMAP)? "IORM" : \
	 (((PMEM_AREA)mem)->eAreaType == PVR_MEM_AREA_IO)? "IO" : \
	 (((PMEM_AREA)mem)->eAreaType == PVR_MEM_AREA_VMALLOC)? "VM" : \
	 (((PMEM_AREA)mem)->eAreaType == PVR_MEM_AREA_ALLOC_PAGES)? "AP" : "ERR")

typedef struct _PVR_MEM_AREA_
{
	union _uData
	{
		struct _sIORemap
		{
		
			IMG_CPU_PHYADDR CPUPhysAddr;
			IMG_VOID *pvIORemapCookie;
		}sIORemap;
		struct _sIO
		{
		
			IMG_CPU_PHYADDR CPUPhysAddr;
		}sIO;
		struct _sVmalloc
		{
		
			IMG_VOID *pvVmallocAddress;
		}sVmalloc;
		struct _sPageList
		{
		
			struct page **pvPageList;
		}sPageList;
	}uData;
	PVR_MEM_AREA_TYPE			eAreaType;
	IMG_UINT32					ui32ByteSize;
	
}MEM_AREA, *PMEM_AREA;

PVRSRV_ERROR	LinuxMMInit(IMG_VOID);
IMG_VOID		LinuxMemAreaStructFree(MEM_AREA *psLinuxMemArea);
IMG_VOID		LinuxMMCleanup(IMG_VOID);

IMG_CPU_PHYADDR  MEM_AREA_ToCpuPAddr(PMEM_AREA psMemArea, IMG_UINT32 ui32ByteOffset);
IMG_VOID		*MEM_AREA_ToCpuVAddr(PMEM_AREA psLinuxMemArea);
PMEM_AREA		 MEM_AREA_NewAllocPages(IMG_UINT32 ui32Bytes);
PMEM_AREA		 MEM_AREA_VMalloc(IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags);
MEM_AREA		*MEM_AREA_NewIORemap(IMG_CPU_PHYADDR BasePAddr, IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags);
IMG_VOID 		 MEM_AREA_AreaDeepFree(MEM_AREA *psLinuxMemArea);

#define KMemCacheAllocWrapper(psCache, Flags) _KMemCacheAllocWrapper(psCache, Flags, __FILE__, __LINE__)
#define VMallocWrapper(ui32Bytes, ui32AllocFlags) _VMallocWrapper(ui32Bytes, ui32AllocFlags, __FILE__, __LINE__)
#define IORemapWrapper(base, size, flag) _IORemapWrapper(base, size, flag, __FILE__, __LINE__);

struct page		*ConvertKVToPage(IMG_VOID *pvAddr);
IMG_VOID		 KMemCacheDestroyWrapper(LinuxKMemCache *psCache);
IMG_VOID		 IOUnmapWrapper(IMG_VOID *pvIORemapCookie);
IMG_VOID		 KMemCacheFreeWrapper(LinuxKMemCache *psCache, IMG_VOID *pvObject);
LinuxKMemCache	*KMemCacheCreateWrapper(IMG_CHAR *pszName, size_t Size, size_t Align, IMG_UINT32 ui32Flags);
IMG_VOID		 VFreeWrapper(IMG_VOID *pvCpuVAddr);
IMG_VOID		*_KMemCacheAllocWrapper(LinuxKMemCache *psCache, gfp_t Flags, IMG_CHAR *pszFileName, IMG_UINT32 ui32Line);
IMG_VOID		*_VMallocWrapper(IMG_UINT32 ui32Bytes, IMG_UINT32 ui32AllocFlags, IMG_CHAR *pszFileName, IMG_UINT32 ui32Line);
IMG_VOID		*_IORemapWrapper(IMG_CPU_PHYADDR BasePAddr, IMG_UINT32 ui32Bytes, IMG_UINT32 ui32MappingFlags,
						  IMG_CHAR *pszFileName, IMG_UINT32 ui32Line);
#endif 
