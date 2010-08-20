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
 * \file cgt/base/stack.h
 * \brief Contains definition of stack container (based on list container) for general use.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-15 00:35:58 +0100 (Tue, 15 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 107 $
 */

#ifndef __CGTL__CGT_BASE_STACK_H_
#define __CGTL__CGT_BASE_STACK_H_

#include "list.h"


namespace cgt
{
  namespace base
  {
    /*!
     * \class stack
     * \brief A stack container based on cgt::base::list.
     * \author Leandro Costa
     * \date 2009
     *
     * A stack container implemented as a list. It's just a wrapper for
     * the list container that exposes only a few set of methods in the
     * interface.
     */

    template<typename _TpItem>
      class stack : private cgt::base::list<_TpItem>
    {
      private:
        typedef cgt::base::list<_TpItem>  _Base;

      public:
        using _Base::find;
        using _Base::clear;
        using _Base::size;
        using _Base::empty;
        using _Base::iterator;
        using _Base::const_iterator;
        using _Base::begin;
        using _Base::end;

      public:
        void insert (const _TpItem &_item) { _Base::push_front (_item); }
        void push (const _TpItem &_item) { _Base::push_front (_item); }
        _TpItem* pop () { return _Base::_pop (_Base::_head); }
        _TpItem* top () { return _Base::_get (_Base::_head); }
    };
  }
}

#endif // __CGTL__CGT_BASE_STACK_H_
