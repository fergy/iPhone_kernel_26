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

#if !defined(AUTOCONF_INCLUDED)
#include <linux/autoconf.h>
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
   #include <linux/modversions.h>
   #define MODVERSIONS
#endif
#endif

#include <linux/version.h>
#include <linux/module.h>
#include <linux/fb.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#else
#include <asm/io.h>
#endif
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/interrupt.h>

#ifdef CONFIG_DEVFS_FS
#include "linux/devfs_fs_kernel.h"
#endif

#include <asm/uaccess.h>
#include <mach/iphone-spi.h>
#include "img_defs.h"
#include "servicesext.h"
#include "kerneldisplay.h"

#include "mbxdisplay.h"

#include "iphonelcdif.h"
#include "iphonelcd_int.h"

#define DRVNAME "iphonelcd"


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
  #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_page_range(dst,src,size,prot))
  #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_page_range(dst,src,size,prot))
#else
  #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
  
    #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_page_range(vma,dst,src,size,prot))
    #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_page_range(vma,dst,src,size,prot))
  #else
  
    #define IMG_IO_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (io_remap_pfn_range(vma,dst,(src>>PAGE_SHIFT),size,prot))
    #define IMG_REMAP_PAGE_RANGE(vma,dst,src,size,prot) (remap_pfn_range(vma,dst,(src>>PAGE_SHIFT),size,prot))
  #endif
#endif

IMG_UINT32 gui32BufferCount = 0x0000;

MODULE_AUTHOR("Imagination Technologies Ltd. <gpl-support@imgtec.com>");
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(depends, "MBX");

#define COPY_TO_USER(arg, addr, size) \
	if(copy_to_user(arg, addr, size) != 0) { \
		printk("IPHONELCD: Copy to user failed."); \
	}

#define COPY_FROM_USER(arg, addr, size) \
	if(copy_from_user(arg, addr, size) != 0) { \
		printk("IPHONELCD: Copy from user failed."); \
	}

MODULE_SUPPORTED_DEVICE(DRVNAME);

#if defined(CONFIG_IPHONE_3G) || defined(CONFIG_IPHONE_2G)
static long fbsize = 320*480*2;
#else
static long fbsize = 240*320*2;
#endif

static long flipbuffers[3] = {0, 0, 0};
static int num_buffers = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#define irqreturn_t void
#define IRQ_HANDLED

MODULE_PARM(fbsize, "l");
MODULE_PARM_DESC(fbsize, "Sets the size of the buffers in a flip chain (default=153600)");
MODULE_PARM(flipbuffers, "1-3l");
MODULE_PARM_DESC(flipbuffers, "Sets the address of the buffers in a flip chain (default=0 -> allocated)");

#else
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))

module_param(fbsize, long, 0);
MODULE_PARM_DESC(fbsize, "Sets the size of the buffers in a flip chain (default=153600)");
module_param_array(flipbuffers, long, num_buffers, 0);
MODULE_PARM_DESC(flipbuffers, "Sets the address of the buffers in a flip chain (default=0 -> allocated)");

#else

module_param(fbsize, long, 0);
MODULE_PARM_DESC(fbsize, "Sets the size of the buffers in a flip chain (default=153600)");
module_param_array(flipbuffers, long, 0, 0);
MODULE_PARM_DESC(flipbuffers, "Sets the address of the buffers in a flip chain (default=0 -> allocated)");

#endif
#endif

int IPHONELCDBridgeDispatch(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
int IPHONELCDOpen(struct inode* pInode, struct file* pFile);
int IPHONELCDRelease(struct inode* pInode, struct file* pFile);


static int AssignedMajorNumber = 0;

static IMG_CHAR szClientDrvName[] = "libiphonelcd.so";

#define unref__ __attribute__ ((unused))

static unsigned int *kmalloc_area = IMG_NULL;

static int IPHONELCDMMap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	ret = IMG_IO_REMAP_PAGE_RANGE(vma, 
			vma->vm_start,
			virt_to_phys((void*)((unsigned long)kmalloc_area)),
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot);

	if (ret != 0)
	{
		ret = -EAGAIN;
	}

	return ret;
}

static struct file_operations cldc_fops = {
	owner:THIS_MODULE,
	ioctl:IPHONELCDBridgeDispatch,
	open:IPHONELCDOpen,
	release:IPHONELCDRelease,
	mmap:IPHONELCDMMap
};


int IPHONELCDOpen(struct inode unref__ * pInode, struct file unref__ * pFile)
{
	int Ret = 0;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	MOD_INC_USE_COUNT;
#endif

	return Ret;
}

int IPHONELCDRelease(struct inode unref__ * pInode, struct file unref__ * pFile)
{
	int Ret = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	MOD_DEC_USE_COUNT;
#endif

	return Ret;
}

IMG_CHAR const * GetClientName(void)
{
	return szClientDrvName;
}

static int __init IPHONELCD_Init(void)
{
	AssignedMajorNumber =
		register_chrdev(AssignedMajorNumber, DRVNAME, &cldc_fops);

	printk ("IPHONELCD_Init: major device %d\n", AssignedMajorNumber);

	if (AssignedMajorNumber <= 0)
	{
		printk ("IPHONELCD_Init: unable to get major\n");
		return -EBUSY;
	}

	if(Init() != PVRSRV_OK) {
		unregister_chrdev(AssignedMajorNumber, DRVNAME);
		return -EINVAL;
	}

#if 0 
	
	devfs_mk_cdev(MKDEV(AssignedMajorNumber, 0),
					S_IFCHR|S_IWUSR|S_IRUGO, DRVNAME);
#endif

	return 0;
} 

static void __exit IPHONELCD_Cleanup(void)
{    
	if(Deinit() != PVRSRV_OK)
	{
		printk ("IPHONELCD_Cleanup: can't deinit device\n");
	}
#if 0 
	devfs_remove(DRVNAME);
#endif

	unregister_chrdev(AssignedMajorNumber, DRVNAME);
} 

PVRSRV_ERROR AllocContiguousMemory(IMG_UINT32 ui32Size, IMG_HANDLE unref__ * hMem, IMG_SYS_PHYADDR *pPhysAddr, IMG_CPU_VIRTADDR *pLinAddr)
{
	dma_addr_t dma;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	*pLinAddr = dma_alloc_coherent(NULL, ui32Size, &dma, GFP_KERNEL);
#else
	*pLinAddr = consistent_alloc(GFP_KERNEL | GFP_DMA, ui32Size, &dma);
#endif
	if(*pLinAddr == IMG_NULL)
	{
		printk ("AllocContiguousMemory: unable to allocate contiguous memory of %x\n", ui32Size);
	}
	else
	{
		pPhysAddr->uiAddr = dma;
		return PVRSRV_OK;
	}

	return PVRSRV_ERROR_OUT_OF_MEMORY;
}

PVRSRV_ERROR FreeContiguousMemory(IMG_UINT32 ui32Size, IMG_HANDLE unref__ hMem, IMG_SYS_PHYADDR PhysAddr, IMG_CPU_VIRTADDR LinAddr)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	dma_free_coherent(NULL, ui32Size, LinAddr, (dma_addr_t)PhysAddr.uiAddr);
#else
	consistent_free(LinAddr, ui32Size, (dma_addr_t)PhysAddr.uiAddr);
#endif
	return PVRSRV_OK;
}


PVRSRV_ERROR GetBackBuffer(IPHONELCD_DEVINFO *psDevInfo, IMG_UINT32 ui32Num)
{
	if(num_buffers)
	{
		if(ui32Num + 1 >= (IMG_UINT32)num_buffers )
			return PVRSRV_ERROR_INVALID_PARAMS;

		psDevInfo->asBackBuffers[ui32Num].sSysAddr.u.sContig.uiAddr = flipbuffers[ui32Num + 1];
		psDevInfo->asBackBuffers[ui32Num].sCPUVAddr = ioremap_nocache(flipbuffers[ui32Num + 1], fbsize);

		if(psDevInfo->asBackBuffers[ui32Num].sCPUVAddr)
			return PVRSRV_OK;

		return PVRSRV_ERROR_BAD_MAPPING;

	}
	else
	{
		return AllocContiguousMemory(psDevInfo->sFBInfo.ui32ByteStride * psDevInfo->sFBInfo.ui32Height,
									&psDevInfo->asBackBuffers[ui32Num].hMemChunk,
									&psDevInfo->asBackBuffers[ui32Num].sSysAddr.u.sContig, 
									&psDevInfo->asBackBuffers[ui32Num].sCPUVAddr);
	}
}

PVRSRV_ERROR ReleaseBackBuffer(IPHONELCD_DEVINFO *psDevInfo, IMG_UINT32 ui32Num)
{
	if(num_buffers)
	{
		if(ui32Num + 1 >= (IMG_UINT32)num_buffers )
			return PVRSRV_ERROR_INVALID_PARAMS;

		iounmap(psDevInfo->asBackBuffers[ui32Num].sCPUVAddr);

		return PVRSRV_OK;
	}
	else
	{
		return FreeContiguousMemory(psDevInfo->sFBInfo.ui32ByteStride * psDevInfo->sFBInfo.ui32Height,
									psDevInfo->asBackBuffers[ui32Num].hMemChunk,
									psDevInfo->asBackBuffers[ui32Num].sSysAddr.u.sContig, 
									psDevInfo->asBackBuffers[ui32Num].sCPUVAddr);
	}

}

IMG_VOID *AllocKernelMem(IMG_UINT32 ui32Size)
{
	return kmalloc(ui32Size, GFP_KERNEL);
}

IMG_VOID FreeKernelMem(IMG_VOID *pvMem)
{
	kfree(pvMem);
}

PVRSRV_ERROR OpenPVRServices (IMG_CHAR *szPVRKernelServicesName, IMG_HANDLE *phPVRServices)
{
	
	PVR_UNREFERENCED_PARAMETER(szPVRKernelServicesName);
	*phPVRServices = 0;
	return PVRSRV_OK;
}


PVRSRV_ERROR ClosePVRServices (IMG_HANDLE unref__ hPVRServices)
{
	
	return PVRSRV_OK;
}


PVRSRV_ERROR GetLibFuncAddr (IMG_HANDLE unref__ hExtDrv, IMG_CHAR *szFunctionName, IMG_VOID **ppvFuncAddr)
{
	if(strcmp("PVRGetDisplayClassJTable", szFunctionName) != 0)
		return PVRSRV_ERROR_INVALID_PARAMS;

	
	*ppvFuncAddr = PVRGetDisplayClassJTable;

	return PVRSRV_OK;
}

PVRSRV_ERROR GetSystemSurfaceInfo(IPHONELCD_DEVINFO *psDevInfo)
{
	struct fb_info *fb;
	IPHONELCD_FBINFO *psFBInfo;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	int i;
#endif

	if(num_registered_fb < 1)
		return PVRSRV_ERROR_GENERIC;

	fb = registered_fb[0];
	psFBInfo = &psDevInfo->sFBInfo;

	psFBInfo->sSysAddr.u.sContig.uiAddr = fb->fix.smem_start;
	psFBInfo->sCPUVAddr = fb->screen_base;
	psFBInfo->ui32Width = fb->var.xres;
	psFBInfo->ui32Height = fb->var.yres;
	psFBInfo->ui32ByteStride = fb->var.xres * (fb->var.bits_per_pixel/8);

	if(fb->var.bits_per_pixel == 16)
	{
		if((fb->var.red.length == 5) &&
			(fb->var.green.length == 6) && 
			(fb->var.blue.length == 5) && 
			(fb->var.red.offset == 11) &&
			(fb->var.green.offset == 5) && 
			(fb->var.blue.offset == 0) && 
			(fb->var.red.msb_right == 0))
		{
			psFBInfo->ePixelFormat = PVRSRV_PIXEL_FORMAT_RGB565;
		}
		else
		{
			printk("Unknown FB format\n");
		}
	}
	else
	{
		printk("Unknown FB format\n");
	}
//FIXME!!!!///////////////////////////////////////////////////////////////
/*#if defined(CONFIG_IPHONE_3G) || defined(CONFIG_IPHONE_2G)
	psFBInfo->pvRegs = ioremap(0x48050000, 0x00001000);
	psFBInfo->pvRegsPhys = (IMG_VOID*)(0x48050000);
	psFBInfo->ui32RegsSize = 0x00001000;
#else*/
	psFBInfo->pvRegs = ioremap(fb->fix.mmio_start, fb->fix.mmio_len);
	psFBInfo->pvRegsPhys = (IMG_VOID*)(fb->fix.mmio_start);
	psFBInfo->ui32RegsSize = fb->fix.mmio_len;
//#endif
//////////////////////////////////////////////////////////////////////////
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	for(i=0; i < IPHONELCD_MAX_BACKBUFFERS + 1; i++)
	{
		if(flipbuffers[i])
			num_buffers++;
	}
#endif

	if(num_buffers)
	{	
		psFBInfo->sSysAddr.u.sContig.uiAddr = flipbuffers[0];
		psFBInfo->sCPUVAddr = ioremap_nocache(flipbuffers[0], fbsize);
	}
	else
	{
		printk("No flipbuffers\n");
	}
	

	return PVRSRV_OK;
}


PVRSRV_ERROR ReleaseSystemSurfaceInfo(IPHONELCD_DEVINFO *psDevInfo)
{
	IPHONELCD_FBINFO *psFBInfo = &psDevInfo->sFBInfo;

	if(num_buffers)
	{	
		iounmap(psFBInfo->sCPUVAddr);
	}

	iounmap(psFBInfo->pvRegs);

	return PVRSRV_OK;
}

#if defined(CONFIG_IPHONE_3G)
irqreturn_t vsync_isr(int irq, void *dev_id,
//#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
					  struct pt_regs *regs
//#endif
					 )
{
	IPHONELCD_DEVINFO *psDevInfo = (IPHONELCD_DEVINFO *)dev_id;
	
	VSyncFlip(psDevInfo);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	return IRQ_HANDLED;
#endif
}

#else 
#if defined(CONFIG_IPHONE_3GS)

static void
vsync_isr(void *arg, struct pt_regs *regs)
{
	PVR_UNREFERENCED_PARAMETER(regs);

	IPHONELCD_DEVINFO *psDevInfo = (IPHONELCD_DEVINFO *)arg;
	
	VSyncFlip(psDevInfo);
}

#endif 
#endif 

static int bVsyncIsrInstalled = IMG_FALSE;
PVRSRV_ERROR InstallVsyncISR(IPHONELCD_DEVINFO *psDevInfo)
{
	if(bVsyncIsrInstalled)
	{
		return PVRSRV_OK;
	}
	
	
	
#if defined(CONFIG_IPHONE_3G)
	if(request_irq(IPHONELCD_IRQ, vsync_isr, GPIO_SPI0_CS0_IPHONE, "IPHONELCD Flip", psDevInfo))
	{
		printk("request IPHONELCD IRQ failed");
		return PVRSRV_ERROR_INIT_FAILURE;
	}
#else
//FIXME!!///////////////////////////////////////////////////
#if defined(CONFIG_IPHONE_3GS)
	if (iphone3gs_disp_register_isr(vsync_isr, psDevInfo,
					DISPC_IRQSTATUS_VSYNC)) {
		printk("request IPHONELCD IRQ failed");
		return PVRSRV_ERROR_INIT_FAILURE;
	};
#endif
////////////////////////////////////////////////////////////
#endif
	bVsyncIsrInstalled = IMG_TRUE;
	return PVRSRV_OK;
}

PVRSRV_ERROR UnInstallVsyncISR(IPHONELCD_DEVINFO *psDevInfo)
{
	if(!bVsyncIsrInstalled)
		return PVRSRV_OK;

#if defined(CONFIG_IPHONE_3G)
	free_irq(IPHONELCD_IRQ, psDevInfo);
#else
//FIXME!!//////////////////////////////////////
#if defined(CONFIG_IPHONE_3GS)
	PVR_UNREFERENCED_PARAMETER(psDevInfo);
	iphone3gs_disp_unregister_isr(vsync_isr);
#endif
///////////////////////////////////////////////
#endif
	bVsyncIsrInstalled = IMG_FALSE;
	return PVRSRV_OK;
}



IMG_VOID WriteReg(IMG_VOID *pvAddr, IMG_UINT32 ui32Value)
{
	writel(ui32Value, pvAddr);
}

IMG_UINT32 ReadReg(IMG_VOID *pvAddr)
{
	return readl(pvAddr);
}



int IPHONELCDBridgeDispatch(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	IPHONELCD_BRIDGE_PACKAGE *psBridgePackageUM = (IPHONELCD_BRIDGE_PACKAGE *)arg;
	IPHONELCD_BRIDGE_PACKAGE sBridgePackageKM;
	IMG_BOOL bAccessibleParams;
	int iRetVal = 0;

	PVR_UNREFERENCED_PARAMETER(inode);
	PVR_UNREFERENCED_PARAMETER(file);

	bAccessibleParams = access_ok(VERIFY_WRITE, psBridgePackageUM, sizeof(IPHONELCD_BRIDGE_PACKAGE));

	if (bAccessibleParams)
	{
		COPY_FROM_USER(&sBridgePackageKM, psBridgePackageUM, sizeof(IPHONELCD_BRIDGE_PACKAGE));

		if(sBridgePackageKM.ui32BridgeID != cmd)
		{
			return -EINVAL;
		}
				
		
		switch (cmd) 
		{
			case IPHONELCDIO_GET_CLIENTINFO :
			{
				IPHONELCD_GET_CLIENTINFO_OUT_DATA	sOutData;
			
				sOutData.eError = GetClientInfo(&sOutData.sClientDevInfo);

				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sOutData, sBridgePackageKM.ui32OutBufferSize);
				break;			
			}
			case IPHONELCDIO_GET_DEVINFO :
			{
				IPHONELCD_GET_DEVINFO_OUT_DATA    sOutData;
				IPHONELCD_DEVINFO *psDevInfo;

				psDevInfo = GetAnchorPtr();
				sOutData.sDevInfo = *psDevInfo;

				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sOutData, sBridgePackageKM.ui32OutBufferSize);
				break;            
			}
			case IPHONELCDIO_SET_MODE :
			{
				IPHONELCD_SET_MODE_INOUT_DATA	sInOut;
	
				COPY_FROM_USER(&sInOut, sBridgePackageKM.pvParamIn, sizeof(IPHONELCD_SET_MODE_INOUT_DATA));
	
				sInOut.eError = SetMode((IPHONELCD_DEVINFO *)sInOut.hDevInfo,
										&sInOut.sSysBusAddr,
										&sInOut.sModeInfo);

				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sInOut, sBridgePackageKM.ui32OutBufferSize);
				break;			
			}			
			case IPHONELCDIO_CREATE_SWAPCHAIN :
			{
				IPHONELCD_CREATE_SWAP_CHAIN_INOUT_DATA	sInOut;
	
				COPY_FROM_USER(&sInOut, sBridgePackageKM.pvParamIn, sizeof(IPHONELCD_CREATE_SWAP_CHAIN_INOUT_DATA));
	
								gui32BufferCount = sInOut.ui32BufferCount;

				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sInOut, sBridgePackageKM.ui32OutBufferSize);
				break;			
			}
			case IPHONELCDIO_DESTROY_SWAPCHAIN :
			{
				IPHONELCD_DESTROY_SWAP_CHAIN_INOUT_DATA	sInOut;
	
				COPY_FROM_USER(&sInOut, sBridgePackageKM.pvParamIn, sizeof(IPHONELCD_DESTROY_SWAP_CHAIN_INOUT_DATA));
	
				sInOut.eError = DestroySwapchain((IPHONELCD_SWAP_CHAIN *)sInOut.hSwapChain, IMG_TRUE);

				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sInOut, sBridgePackageKM.ui32OutBufferSize);
				break;			
			}
			case IPHONELCDIO_IS_POWER_DOWN:
			{
				IPHONELCD_IS_POWER_DOWN_OUT_DATA sOut;
				sOut.bResult = IsPowerDown((IPHONELCD_DEVINFO*)GetAnchorPtr());
				COPY_TO_USER(sBridgePackageKM.pvParamOut, &sOut, sBridgePackageKM.ui32OutBufferSize);
				break;
			}
			case IPHONELCDIO_INIT_MAIN:
			{
				IPHONELCD_INIT_MAIN_IN_DATA sIn;
				COPY_FROM_USER(&sIn, sBridgePackageKM.pvParamIn, sizeof(IPHONELCD_INIT_MAIN_IN_DATA));

				InitMain((IMG_VOID*)sIn.pvKernelPageDevInfo);
				break;
			}
			/*case IPHONELCDIO_DEINIT_MAIN:
			{
				DeinitMain();
				break;
			}
			case IPHONEPLCDIO_UNINSTALL_VSYNC_ISR:
			{
				UnInstallVsyncISR(GetAnchorPtr());
				break;
			}*/
			case IPHONELCDIO_INSTALL_VSYNC_ISR:
			{
				InstallVsyncISR(GetAnchorPtr());
			}
			default :
			{
				iRetVal = -EINVAL;
			}
		}
	}
	else
	{
		iRetVal = -EINVAL;
	}
		
	return iRetVal;
}

IMG_BOOL IsPowerDown(IPHONELCD_DEVINFO *psDevInfo)
{

	if(ReadReg((IMG_VOID *)((IMG_UINT8 *)psDevInfo->sFBInfo.pvRegs + IPHONELCD_CONTROL)) & (IPHONE_CONTROL_DIGITALENABLE | IPHONE_CONTROL_LCDENABLE) )
	{
		psDevInfo->bIsPoweredDown=IMG_FALSE;
	}
	else
	{
		psDevInfo->bIsPoweredDown=IMG_TRUE;
	}
	return psDevInfo->bIsPoweredDown;
}

module_init(IPHONELCD_Init);
module_exit(IPHONELCD_Cleanup);

