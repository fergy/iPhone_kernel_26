/*
 *  arch/arm/mach-apple_iphone/include/mach/timex.h
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Copyright (c) 2003-2010 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ASM_ARCH_TIMEX_H
#define __ASM_ARCH_TIMEX_H __FILE__

/* CLOCK_TICK_RATE needs to be evaluatable by the cpp, so making it
 * a variable is useless. It seems as long as we make our timers an
 * exact multiple of HZ, any value that makes a 1->1 correspondence
 * for the time conversion functions to/from jiffies is acceptable.
*/

#define CLOCK_TICK_RATE 12000000

#endif /* __ASM_ARCH_TIMEX_H */
