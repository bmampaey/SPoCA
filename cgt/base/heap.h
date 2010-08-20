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
 * \file cgt/base/heap.h
 * \brief Contains definition of a heap container (based on vector container) for general use.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-12 01:12:07 +0100 (Sat, 12 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 106 $
 */

#ifndef __CGTL__CGT_BASE_HEAP_H_
#define __CGTL__CGT_BASE_HEAP_H_

#include "vector.h"
#include "alloc/allocator.h"


namespace cgt
{
  namespace base
  {
    /*!
     * \class heap
     * \brief A heap container based on vector.
     * \author Leandro Costa
     * \date 2009
     *
     * A heap container based on vector. It's not just a wrapper since
     * it contains some methods that not exist in the vector container,
     * as \b modify and \b modify_by template. The heap container should
     * be used if you wants to maintain the heap invariant always true.
     */

    template<typename _TpItem, template<typename> class _HeapInvariant = cgt::base::_LessThan, typename _Alloc = cgt::base::alloc::_Allocator<_TpItem> >
      class heap : private cgt::base::vector<_TpItem, _HeapInvariant, _Alloc>
    {
      private:
        typedef cgt::base::vector<_TpItem, _HeapInvariant, _Alloc>  _Base;

      private:
        typedef typename _Base::iterator        _Iterator;
        typedef typename _Base::const_iterator  _ConstIterator;

      public:
        using _Base::find;
        using _Base::clear;
        using _Base::size;
        using _Base::empty;
        using _Base::iterator;
        using _Base::const_iterator;
        using _Base::begin;
        using _Base::end;

      private:
        void _rebuild (size_t _pos);

      public:
        void push (const _TpItem& _i) { _Base::push_heap (_i); }
        _TpItem* pop () { return _Base::pop_heap (); }
        void modify (_ConstIterator& _it, const _TpItem& _i);
        void modify (_Iterator& _it, const _TpItem& _i) { modify (reinterpret_cast<_ConstIterator&>(_it), _i); }
 
      public:
        template<typename _Modifier, typename _Parm>
          void modify_by (_ConstIterator& _it, _Modifier _modify, const _Parm& _parm)
          {
            if (_it >= begin () && _it < end ())
            {
              _TpItem* _ptr = &(*(find (*_it)));
              _modify (*_ptr, _parm);
              _rebuild (_it - begin () + 1);
              _it = find (*_ptr);
            }
          }
    };


    template<typename _TpItem, template<typename> class _HeapInvariant, typename _Alloc>
      void heap<_TpItem, _HeapInvariant, _Alloc>::_rebuild (size_t _pos)
      {
        while (_pos > 0)
          _Base::rebuild_heap (--_pos);
      }

    template<typename _TpItem, template<typename> class _HeapInvariant, typename _Alloc>
      void heap<_TpItem, _HeapInvariant, _Alloc>::modify (_ConstIterator& _it, const _TpItem& _i)
      {
        if (_it >= begin () && _it < end ())
        {
          *(find (*_it)) = _i;
          _rebuild (_it - begin () + 1);
        }
      }
  }
}

#endif // __CGTL__CGT_BASE_HEAP_H_
