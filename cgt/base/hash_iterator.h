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
 * \file cgt/base/hash_iterator.h
 * \brief Contains definition of hash iterator (common and const).
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_HASH_ITERATOR_H_
#define __CGTL__CGT_BASE_HASH_ITERATOR_H_

#include "hash_iterator_base.h"


namespace cgt
{
  namespace base
  {
    template<typename _TpKey, typename _TpItem, typename _Alloc>
      class hash;


    /*!
     * \class _HashIterator
     * \brief An iterator for hash container.
     * \author Leandro Costa
     * \date 2009
     *
     * This is the default iterator for hash container.
     */

    template<typename _TpKey, typename _TpItem, typename _Alloc, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
      class _HashIterator : public _HashIteratorBase<_TpKey, _TpItem, _Alloc, _TpIterator>
    {
      private:
        typedef _HashItem<pair<const _TpKey, _TpItem> >                                 _Item;
        typedef _HashIteratorBase<_TpKey, _TpItem, _Alloc, _TpIterator>                 _Base;
        typedef _HashIterator<_TpKey, _TpItem, _Alloc, _TpIterator>                     _Self;
        typedef _HashIterator<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpCommon>  _SelfCommon;
        typedef hash<_TpKey, _TpItem, _Alloc>                                           _Hash;

      private:
        typedef typename _TpIterator<pair<const _TpKey, _TpItem> >::pointer   pointer;
        typedef typename _TpIterator<pair<const _TpKey, _TpItem> >::reference reference;

      private:
        using _Base::_ptr;
        using _Base::_ptr_hash;

      public:
        _HashIterator () : _Base () { }
        _HashIterator (_Item* _p, _Hash* _p_hash) : _Base (_p, _p_hash) { }
        _HashIterator (const _SelfCommon& _it) : _Base (_it) { }
        virtual ~_HashIterator () { }

      private:
        void _incr ()
        {
          if (_ptr->_next)
            _ptr = _ptr->_next;
          else
          {
            size_t _pos = _ptr_hash->_get_position (_ptr->_item.first) + 1;

            while (_pos < _ptr_hash->_tabsize && ! _ptr_hash->_table [_pos])
              _pos++;

            if (_pos < _ptr_hash->_tabsize)
              _ptr = _ptr_hash->_table [_pos];
            else
              _ptr = NULL;
          }
        };

      public:
        reference operator*() const { return _ptr->_item; }
        pointer operator->() const { return &(operator*()); }

        _Self& operator++() { _incr (); return *this; }
        const _Self operator++(int) { _Self _s = *this; _incr (); return _s; }
    };
  }
}

#endif // __CGTL__CGT_BASE_HASH_ITERATOR_H_
