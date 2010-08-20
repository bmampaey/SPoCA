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
 * \file cgt/base/hash_func.h
 * \brief Contains definition of the hash function.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-24 01:55:40 +0100 (Tue, 24 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 81 $
 */

#ifndef __CGTL__CGT_BASE_HASH_FUNC_H_
#define __CGTL__CGT_BASE_HASH_FUNC_H_


namespace cgt
{
  namespace base
  {
    /*!
     * \struct _HashFunc
     * \brief The hash-function used by hash for general use.
     * \author Leandro Costa
     * \date 2009
     * \todo Define \b operator() for all numerical types as a simple mod operation.
     */

    template<typename _TpKey>
      struct _HashFunc
      {
        const size_t operator()(const _TpKey& _key) const;
      };

    template<typename _TpKey>
      const size_t _HashFunc<_TpKey>::operator()(const _TpKey& _key) const
      {
        size_t _h = 0;

        size_t numbytes = sizeof (_TpKey);

        for (size_t i = 0; i < numbytes; i++)
          _h = 5*_h + *(reinterpret_cast<const unsigned char *>(&_key)+i);

        return _h;
      }

    template<>
      const size_t _HashFunc<int>::operator()(const int& _key) const
      {
        return _key;
      }
  }
}

#endif // __CGTL__CGT_BASE_HASH_FUNC_H_
