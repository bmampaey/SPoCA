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
 * \file cgt/base/pair.h
 * \brief Contains definition of a pair container for general use.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_PAIR_H_
#define __CGTL__CGT_BASE_PAIR_H_


namespace cgt
{
  namespace base
  {
    /*!
     * \class pair
     * \brief A simple STL-like pair container with two elements: first and second.
     * \author Leandro Costa
     * \date 2009
     *
     * A simple STL-like pair container with just two public elements: first and second.
     */

    template<typename _Tp1, typename _Tp2>
      class pair
      {
        public:
          pair (const _Tp1& _i1, const _Tp2& _i2) : first (_i1), second (_i2) { }

        public:
          _Tp1 first;
          _Tp2 second;
      };
  }
}

#endif // __CGTL__CGT_BASE_PAIR_H_
