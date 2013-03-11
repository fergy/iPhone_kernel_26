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

#ifndef __MBX_ACCESS_USER_H__
#define __MBX_ACCESS_USER_H__

#define MBXACCESS_BASE 'M'

#define MBXDebug          _IO(MBXACCESS_BASE, 0)
#define MBXCallAsyncKernelService    _IOR(MBXACCESS_BASE, 1, int)
#define MBXCallSyncKernelService    _IOR(MBXACCESS_BASE, 2, int)
#define MBXFailed   _IOR(MBXACCESS_BASE, 3, int)
#define MBXInitDrv  _IOR(MBXACCESS_BASE, 4, int)
#define MBXDeInitDrv _IOR(MBXACCESS_BASE, 5, int)
#define MBXOpenComplete  _IOR(MBXACCESS_BASE, 6, int)
#define MBXCloseComplete _IOR(MBXACCESS_BASE, 7, int)
#define MBXRecyleDriver _IOR(MBXACCESS_BASE, 8, int)

#endif 


