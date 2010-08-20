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
 * \file cgt/misc/cxxtest_defs.h
 * \brief Contains de definition of a macro to insert break points into the source code.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-26 21:31:35 -0200 (Thu, 26 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 86 $
 */

#ifndef __CGTL__CGT_MISC_CXXTEST_DEFS_H_
#define __CGTL__CGT_MISC_CXXTEST_DEFS_H_

//#ifdef USE_UT_CXXTEST
//#define UT_CXXTEST_VIRTUAL virtual
//#else
//#define UT_CXXTEST_VIRTUAL
//#endif

//#ifdef USE_UT_CXXTEST
//#define UT_CXXTEST_FRIEND_CLASS(C) friend class C;
//#else
//#define UT_CXXTEST_FRIEND_CLASS(C)
//#endif

#ifdef USE_UT_CXXTEST
#define UT_CXXTEST_DEFINE_CLASS(C) class C
#else
#define UT_CXXTEST_DEFINE_CLASS(C)
#endif

#ifdef USE_UT_CXXTEST
#define UT_CXXTEST_FRIEND_CLASS(C) friend class C
#else
#define UT_CXXTEST_FRIEND_CLASS(C)
#endif

#endif // __CGTL__CGT_MISC_CXXTEST_DEFS_H_
