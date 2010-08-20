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
 * \file cgt/base/hash.h
 * \brief Contains a general-purpose hash container.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-11 02:54:25 +0100 (Fri, 11 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 105 $
 */

#ifndef __CGTL__CGT_BASE_HASH_H_
#define __CGTL__CGT_BASE_HASH_H_

#include <string.h>
#include "hash_item.h"
#include "hash_iterator.h"
#include "alloc/allocator.h"
#include "hash_func.h"
#include "pair.h"


namespace cgt
{
  namespace base
  {
    /*!
     * \class hash
     * \brief A general-purpose hash container, 
     * \author Leandro Costa
     * \date 2009
     *
     * This is a general-purpose hash container, table size starts with 2 and
     * increases exponentially. The hash function is _HashFunc, and each item is
     * represented by a _HashItem structure, that contains key and data to be stored.
     * Table increases when the number of elements of the hash is equal to the table
     * size, and it happens in time complexity \b O(n) (worst case), where \b n is the
     * number of elements stored in the hash.
     */

    template<typename _TpKey, typename _TpItem, typename _Alloc = cgt::base::alloc::_Allocator<_HashItem<pair<const _TpKey, _TpItem> > > >
      class hash
      {
        private:
          friend class _HashIterator<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpCommon>;
          friend class _HashIterator<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpConst>;

        private:
          typedef hash<_TpKey, _TpItem, _Alloc>           _Self;
          typedef _HashItem<pair<const _TpKey, _TpItem> > _Item;
          typedef _HashFunc<_TpKey>                       _Func;

        private:
          typedef typename _Alloc::template rebind<_Item>::other allocator_type;

        public:
          typedef _HashIterator<_TpKey, _TpItem, _Alloc>            iterator;
          typedef _HashIterator<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpConst>  const_iterator;

        public:
          hash () : _size (0), _tabsize (2) { _init (); }
          hash (const hash& _h) : _table (NULL), _size (0), _tabsize (0) { *this = _h; }
          virtual ~hash () { _remove_all (); free (_table); }

        public:
          _Self& operator=(const _Self& _s);

        private:
          void _init ();
          void _increase ();
          void _insert (_Item* const _p);
          _Item* _pop (_Item** const _p);
          void _remove_all ();
          const size_t _get_position (const _TpKey& _key) const;
          _TpItem** _get_head ();

        public:
          void insert (const _TpKey& _key, const _TpItem& _item);
          size_t size () const { return _size; }
          const bool empty () const { return (! _size); }

        public:
          _TpItem* operator[](const _TpKey& _key);

        public:
          iterator begin ();
          const_iterator begin () const;
          iterator end () { return iterator (NULL, this); }
          const_iterator end () const { return const_iterator (NULL, this); }

        private:
          _Item** _table;
          size_t  _size;
          size_t  _tabsize;

          _Func           _hash_func;
          allocator_type  _alloc;
      };


    template<typename _TpKey, typename _TpItem, typename _Alloc>
      hash<_TpKey, _TpItem, _Alloc>& hash<_TpKey, _TpItem, _Alloc>::operator=(const _Self& _s)
      {
        if (_table)
        {
          _remove_all ();
          free (_table);
        }

        _size     = 0;
        _tabsize  = _s._tabsize;

        _table = (_Item **) malloc (_tabsize * sizeof (_Item **));
        bzero (_table, _tabsize * sizeof (_Item **));

        for (size_t i = 0; i < _s._tabsize; i++)
        {
          _Item** _ptr = &(_s._table [i]);

          while (*_ptr)
          {
            insert ((*_ptr)->_item.first, (*_ptr)->_item.second);
            _ptr = &((*_ptr)->_next);
          }
        }

        return *this;
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      void hash<_TpKey, _TpItem, _Alloc>::_init ()
      {
        _table = (_Item **) malloc (_tabsize * sizeof (_Item **));
        bzero (_table, _tabsize * sizeof (_Item **));
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      void hash<_TpKey, _TpItem, _Alloc>::_increase ()
      {
        _tabsize *= 2;
        _table = (_Item **) realloc (_table, _tabsize * sizeof (_Item **));
        bzero (&(_table [_size]), (_tabsize/2) * sizeof (_Item **));

        for (size_t i = 0; i < _tabsize/2; i++)
        {
          _Item** _ptr = &(_table [i]);

          while (*_ptr)
          {
            if (_get_position ((*_ptr)->_item.first) != i)
              _insert (_pop (_ptr));
            else
              _ptr = &((*_ptr)->_next);
          }
        }
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      void hash<_TpKey, _TpItem, _Alloc>::_insert (_Item* const _p)
      {
        _Item** _ptr = &(_table [_get_position (_p->_item.first)]);
        while (*_ptr)
          _ptr = &((*_ptr)->_next);
        *_ptr = _p;

        _size++;
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      _HashItem<pair<const _TpKey, _TpItem> >* hash<_TpKey, _TpItem, _Alloc>::_pop (_Item** const _p)
      {
        _Item* _ptr = *_p;
        *_p = (*_p)->_next;
        _ptr->_next = NULL;
        _size--;

        return _ptr;
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      void hash<_TpKey, _TpItem, _Alloc>::_remove_all ()
      {
        for (size_t i = 0; i < _tabsize; i++)
        {
          while (_table [i])
          {
            _Item* _ptr = _table [i];
            _table [i] = _table [i]->_next;
            _alloc.destroy (_ptr);
            _alloc.deallocate (_ptr, 1);
          }
        }
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      const size_t hash<_TpKey, _TpItem, _Alloc>::_get_position (const _TpKey& _key) const
      {
        return (_hash_func (_key) % _tabsize);
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      _TpItem** hash<_TpKey, _TpItem, _Alloc>::_get_head ()
      {
        _TpItem** _ptr = &(_table [0]);

        while (! *_ptr)
          _ptr++;

        return _ptr;
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      void hash<_TpKey, _TpItem, _Alloc>::insert (const _TpKey& _key, const _TpItem& _item)
      {
        if (_size == _tabsize)
          _increase ();

        _Item* _ptr = _alloc.allocate (1);
        _alloc.construct (_ptr, _Item (pair<const _TpKey, _TpItem> (_key, _item)));
        _insert (_ptr);
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      _TpItem* hash<_TpKey, _TpItem, _Alloc>::operator[](const _TpKey& _key)
      {
        _TpItem* _ptr = NULL;

        _Item* _p = _table [_get_position (_key)];

        while (_p && _p->_item.first != _key)
          _p = _p->_next;

        if (_p)
          _ptr = &(_p->_item.second);

        return _ptr;
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      _HashIterator<_TpKey, _TpItem, _Alloc> hash<_TpKey, _TpItem, _Alloc>::begin ()
      {
        size_t _pos = 0;

        while (_pos < _tabsize && ! _table [_pos])
          _pos++;

        if (_pos < _tabsize)
          return iterator (_table [_pos], this);
        else
          return end ();
      }

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      _HashIterator<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpConst> hash<_TpKey, _TpItem, _Alloc>::begin () const
      {
        size_t _pos = 0;

        while (! _table [_pos] && _pos < _tabsize)
          _pos++;

        if (_pos < _tabsize)
          return iterator (_table [_pos], this);
        else
          return end ();
      }
  }
}


#endif
