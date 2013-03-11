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

#ifndef _SERVICES_HEADERS_H_
#define _SERVICES_HEADERS_H_

#ifdef DEBUG_RELEASE_BUILD
#pragma optimize( "", off )
#define DEBUG		1
#endif

#include "img_defs.h"
#include "services.h"
#include "servicesint.h"


#include "resman.h"
#if defined(UBUILD)
#include "power.h"
#include "queue.h"
#include "metrics.h"
#include "env.h"
#include "pdump2.h"
#endif

#include "syscommon.h"
#include "pvr_debug.h"
#include "hostfunc.h"


#if defined(PDUMP2)

#define SRVKM_WRITE_HW_REG(pvRegs, addr, val)      WriteHWReg(pvRegs, addr, val);      PDUMP2REG(addr, val); 
#define SRVKM_WRITE_HW_REGS(pvRegs, count, psRegs) WriteHWRegs(pvRegs, count, psRegs); PDUMP2REGARRAY(0, psRegs, count); 

#else
	
#define SRVKM_WRITE_HW_REG(pvRegs, addr, val)       WriteHWReg(pvRegs, addr, val)
#define SRVKM_WRITE_HW_REGS(pvRegs, count, psRegs)  WriteHWRegs(pvRegs, count, psRegs)

#endif  

#define SRVKM_WRITE_HW_REG_NOPD(pvRegs, addr, val)      WriteHWReg(pvRegs, addr, val);       
#define SRVKM_WRITE_HW_REGS_NOPD(pvRegs, count, psRegs) WriteHWRegs(pvRegs, count, psRegs);  

#endif 

