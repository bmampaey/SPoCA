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
 * \file cgt/base/hash_item.h
 * \brief Contains definition of the strucutre used as a hash's item.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_HASH_ITEM_H_
#define __CGTL__CGT_BASE_HASH_ITEM_H_


namespace cgt
{
  namespace base
  {
    /*!
     * \class _HashItem
     * \brief The item of a hash. It contains the key and the data for a hash's element.
     * \author Leandro Costa
     * \date 2009
     *
     * This is the item of a hash. It contains the key and the data for a
     * hash's element.
     */

    template<typename _TpItem>
      class _HashItem
      {
        public:
          _HashItem (const _TpItem& _i) : _item (_i), _next (NULL) { }

        public:
          _TpItem     _item;
          _HashItem*  _next;
      };
  }
}

#endif // __CGTL__CGT_BASE_HASH_ITEM_H_
