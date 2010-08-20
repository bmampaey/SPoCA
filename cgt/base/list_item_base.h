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
 * \file cgt/base/list_item_base.h
 * \brief Contains the basic definition of a list item (structure with just the pointers).
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_LIST_ITEM_BASE_H_
#define __CGTL__CGT_BASE_LIST_ITEM_BASE_H_

//#include "iterator/iterator_type.h"


namespace cgt
{
  namespace base
  {
    template<typename _TpItem>
      class _ListItem;

    /*!
     * \class _ListItemBase
     * \brief The base for an item of a list. It contains the pointers to next previous elements.
     * \author Leandro Costa
     * \date 2009
     */

    template<typename _TpItem>
      class _ListItemBase
      {
        private:
          friend class _ListItem<_TpItem>;

        private:
          typedef _ListItemBase<_TpItem> _Self;

        private:
          _ListItemBase () : _next (NULL), _prev (NULL) { }

        public:
          _Self* _next;
          _Self* _prev;
      };
  }
}

#endif // __CGTL__CGT_BASE_LIST_ITEM_BASE_H_
