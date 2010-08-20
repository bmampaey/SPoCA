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
 * \file cgt/base/compares.h
 * \brief Contains definition of comparison operators
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-23 22:55:40 -0200 (Mon, 23 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 81 $
 */

#ifndef __CGTL__CGT_BASE_COMPARES_H_
#define __CGTL__CGT_BASE_COMPARES_H_


namespace cgt
{
  namespace base
  {
    /*!
     * \struct _LessThan
     * \brief Operator less than (<) used as heap invariant.
     * \author Leandro Costa
     * \date 2009
     */

    template<typename _Tp>
      struct _LessThan { const bool operator() (const _Tp& _x, const _Tp& _y) const { return (_x < _y); } };

    /*!
     * \struct _LessEqualsTo
     * \brief Operator less or equals to (<=) used as heap invariant.
     * \author Leandro Costa
     * \date 2009
     */

    template<typename _Tp>
      struct _LessEqualsTo { const bool operator() (const _Tp& _x, const _Tp& _y) const { return (_x <= _y); } };

    /*!
     * \struct _GreaterThan
     * \brief Operator greater than (>) used as heap invariant.
     * \author Leandro Costa
     * \date 2009
     */

    template<typename _Tp>
      struct _GreaterThan { const bool operator() (const _Tp& _x, const _Tp& _y) const { return (_x > _y); } };

    /*!
     * \struct _GreaterEqualsTo
     * \brief Operator greater or equals to (>=) used as heap invariant.
     * \author Leandro Costa
     * \date 2009
     */

    template<typename _Tp>
      struct _GreaterEqualsTo { const bool operator() (const _Tp& _x, const _Tp& _y) const { return (_x >= _y); } };
  }
}

#endif // __CGTL__CGT_BASE_COMPARES_H_
