/*
 * CGTL - A graph template library for C++
 * ---------------------------------------
 * Copyright (C) 2009 Leandro Costa
 *
 * This file is part of CGTL.
 *
 * CGTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CGTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CGTL. If not, see <http://www.gnu.org/licenses/>.
 */

/*!
 * \file cgt/misc/debug.h
 * \brief Contains de definition of a macro to insert break points into the source code.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-03 02:26:39 +0100 (Thu, 03 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 98 $
 */

#ifndef __CGTL__CGT_MISC_DEBUG_H_
#define __CGTL__CGT_MISC_DEBUG_H_

#if (defined DEBUG)
#  define _BRK() { asm ("int $3"); } // __GNUC__
#elif !defined (_BRK)
#  define _BRK() {;}
#endif

#endif // __CGTL__CGT_MISC_DEBUG_H_
