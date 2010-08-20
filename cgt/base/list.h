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
 * \file cgt/base/list.h
 * \brief Contains definition of a doubly-linked list container for general use.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-11 01:05:43 +0100 (Fri, 11 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 100 $
 */

#ifndef __CGTL__CGT_BASE_LIST_H_
#define __CGTL__CGT_BASE_LIST_H_

#include "list_item.h"
#include "list_iterator.h"
#include "alloc/allocator.h"


namespace cgt
{
  /*!
   * \namespace cgt::base
   * \brief The namespace where are defined structures for general propose
   *        as list, hash, vector, stack, queue, pair.
   */
  namespace base
  {
    /*!
     * \class _List
     * \brief A doubly-linked list container.
     * \author Leandro Costa
     * \date 2009
     *
     * A doubly-linked list container. This is the \b private class.
     *
     * Each list has two pointers and an attribute of type size_t. For each item
     * inserted in the list we need two more pointers to doubly-link the item.
     * So, the overhead of a list in a 32-bytes architecture is <b>12 + 8n bytes</b>,
     * where \b n is the number of items. The total size of the list is
     * <b>12 + n * (8 + sizeof (_TpItem)) bytes</b>.
     *
     * \code
     *  ________________________________
     * |          _List's size          |
     * |________________________________|
     * | 12 + n * (8 + sizeof(_TpItem)) |
     * |________________________________|
     *
     * \endcode
     */

    template<typename _TpItem, typename _Alloc>
      class _List
      {
        private:
          typedef _List<_TpItem, _Alloc>  _Self;
          typedef _ListItem<_TpItem>      _Item;

        public:
          typedef _ListIterator<_TpItem>                                iterator;
          typedef _ListIterator<_TpItem, cgt::base::iterator::_TpConst> const_iterator;

        private:
          typedef typename _Alloc::template rebind<_Item>::other allocator_type;

        public:
          _List () : _head (NULL), _tail (NULL), _size (0) { }
          _List (const _List& _l) : _head (NULL), _tail (NULL), _size (0) { *this = _l; }
          virtual ~_List () { _remove_all (); }

        public:
          _Self& operator=(const _Self& _l);

        private:
          _Item* _allocate (const _TpItem& _item);
          void _deallocate (_Item* const _ptr);
          _TpItem& _push_back (_Item* _ptr);
          _TpItem& _push_front (_Item* _ptr);
          void _unlink (const _Item* const _ptr);
          void _rebuild_heap (unsigned int i, _TpItem* _arrayItem[]);

        protected:
          _TpItem& _push_front (const _TpItem& _item);
          _TpItem& _push_back (const _TpItem& _item);
          _TpItem* _pop (_Item* _ptr_item);
          _TpItem* _get (_Item* _ptr_item) const;
          _Item* _find (const _TpItem& _item) const;
          void _remove (_Item* _ptr);
          void _remove_all ();

        public:
          void make_heap ();
          _TpItem* pop_heap ();

        public:
          iterator begin () { return iterator (_head); }
          iterator end () { return iterator (NULL); }
          const_iterator begin () const { return const_iterator (_head); }
          const_iterator end () const { return const_iterator (NULL); }

        public:
          static void swap (_Self& _list1, _Self& _list2);

        private:
          allocator_type  _alloc;

        protected:
          _Item*  _head;
          _Item*  _tail;
          size_t  _size;
      };

    template<typename _TpItem, typename _Alloc>
      _List<_TpItem, _Alloc>& _List<_TpItem, _Alloc>::operator=(const _Self& _l)
      {
        _remove_all ();

        const_iterator _itEnd = _l.end ();
        for (const_iterator _it = _l.begin (); _it != _itEnd; ++_it)
          _push_back (*_it);

        return *this;
      }

    template<typename _TpItem, typename _Alloc>
      _ListItem<_TpItem>* _List<_TpItem, _Alloc>::_allocate (const _TpItem& _item)
      {
        _Item* _ptr = _alloc.allocate (1);
        _alloc.construct (_ptr, _Item (_item));
        return _ptr;
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::_deallocate (_Item* const _ptr)
      {
        _alloc.destroy (_ptr);
        _alloc.deallocate (_ptr, 1);
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem& _List<_TpItem, _Alloc>::_push_back (const _TpItem& _item)
      {
        return _push_back (_allocate (_item));
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem& _List<_TpItem, _Alloc>::_push_back (_Item *_ptr)
      {
        if (! _head)
          _head = _ptr;
        else
        {
          _ptr->_prev = _tail;
          _tail->_next = _ptr;
        }

        _tail = _ptr;
        _size++;

        return _ptr->_data;
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem& _List<_TpItem, _Alloc>::_push_front (const _TpItem& _item)
      {
        return _push_front (_allocate (_item));
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem& _List<_TpItem, _Alloc>::_push_front (_Item *_ptr)
      {
        if (! _tail)
          _tail = _ptr;
        else
        {
          _ptr->_next = _head;
          _head->_prev = _ptr;
        }

        _head = _ptr;
        _size++;

        return _ptr->_data;
      }

    template<typename _TpItem, typename _Alloc>
      _ListItem<_TpItem>* _List<_TpItem, _Alloc>::_find (const _TpItem& _item) const
      {
        _Item* _ptr = _head;

        while (_ptr != NULL && _ptr->_data != _item)
          _ptr = static_cast<_Item *>(_ptr->_next);

        return _ptr;
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem* _List<_TpItem, _Alloc>::_get (_Item* _ptr_item) const
      {
        _TpItem* _ptr = NULL;

        if (_ptr_item)
          _ptr = &(_ptr_item->_data);

        return _ptr;
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem* _List<_TpItem, _Alloc>::_pop (_Item* _ptr_item)
      {
        _TpItem* _ptr = NULL;

        if (_ptr_item)
        {
          _unlink (_ptr_item);
          _ptr = new _TpItem (_ptr_item->_data);
          _deallocate (_ptr_item);
        }

        return _ptr;
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::_unlink (const _Item* const _ptr)
      {
        if (_ptr->_prev)
          _ptr->_prev->_next = _ptr->_next;
        else
          _head = static_cast<_Item *>(_ptr->_next);

        if (_ptr->_next)
          _ptr->_next->_prev = _ptr->_prev;
        else
          _tail = static_cast<_Item *>(_ptr->_prev);

        _size--;
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::_remove (_Item* _ptr)
      {
        if (_ptr)
        {
          _unlink (_ptr);
          _deallocate (_ptr);
        }
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::_remove_all ()
      {
        while (_head)
          _remove (_head);
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::make_heap ()
      {
        _TpItem* _arrayItem [_size];

        iterator it;
        iterator itEnd = end ();
        int i = 0;

        for (it = begin (); it != itEnd; ++it)
          _arrayItem [i++] = &(*it);

        i = _size/2 - 1;

        while (i >= 0)
          _rebuild_heap (i--, _arrayItem);
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::_rebuild_heap (unsigned int i, _TpItem* _arrayItem[])
      {
        _TpItem item = *(_arrayItem [i]);

        unsigned int k = 2*i+1;

        while (k < _size)
        {
          if (k+1 < _size && *(_arrayItem[k+1]) < *(_arrayItem[k]))
            k++;

          if (*(_arrayItem [k]) < item)
          {
            *(_arrayItem [i]) = *(_arrayItem [k]);
            i = k;
            k = 2*i+1;
          }
          else
            break;
        }

        *(_arrayItem [i]) = item;
      }

    template<typename _TpItem, typename _Alloc>
      _TpItem* _List<_TpItem, _Alloc>::pop_heap ()
      {
        _TpItem* _ptr = NULL;

        if (_head)
        {
          if (_head == _tail)
            _ptr = _pop (_head);
          else
          {
            _ptr = new _TpItem (_head->_data);

            _TpItem* _arrayItem [_size];

            iterator it;
            iterator itEnd = end ();
            int i = 0;

            for (it = begin (); it != itEnd; ++it)
              _arrayItem [i++] = &(*it);

            _TpItem* _p = _pop (_tail);
            _head->_data = *_p;
            delete _p;

            _rebuild_heap (0, _arrayItem);
          }
        }

        return _ptr;
      }

    template<typename _TpItem, typename _Alloc>
      void _List<_TpItem, _Alloc>::swap (_Self& _list1, _Self& _list2)
      {
        _Item* _p = _list1._head;
        _list1._head = _list2._head;
        _list2._head = _p;

        _p = _list1._tail;
        _list1._tail = _list2._tail;
        _list2._tail = _p;

        size_t _s = _list1._size;
        _list1._size = _list2._size;
        _list2._size = _s;
      }

    /*!
     * \class list
     * \brief A doubly-linked list container.
     * \author Leandro Costa
     * \date 2009
     *
     * A doubly-linked list container. This is the \b public interface.
     */

    template<typename _TpItem, typename _Alloc = cgt::base::alloc::_Allocator<_ListItem<_TpItem> > >
      class list : public _List<_TpItem, _Alloc>
    {
      private:
        typedef _List<_TpItem, _Alloc> _Base;

      public:
        list () : _Base () { }
        list (const list& _l) : _Base (_l) { }

      public:
        _TpItem& insert (const _TpItem& _item) { return _Base::_push_back (_item); }
        _TpItem& push_front (const _TpItem& _item) { return _Base::_push_front (_item); }
        _TpItem& push_back (const _TpItem& _item) { return _Base::_push_back (_item); }
        _TpItem* pop_front () { return _pop (_Base::_head); }
        _TpItem* pop_back () {return _pop (_Base::_tail); }
        _TpItem* front () { return _get (_Base::_head); }
        const _TpItem* const front () const { return _get (_Base::_head); }
        _TpItem* back () {return _get (_Base::_tail); }
        const _TpItem* const back () const { return _get (_Base::_tail); }
        void remove (const _TpItem& _item) { _Base::_remove (_Base::_find (_item)); }
        void clear () { _Base::_remove_all (); }
        const size_t size () const { return _Base::_size; }
        const bool empty () const { return (! _Base::_size); }
        typename _Base::iterator find (const _TpItem& _item) { return typename _Base::iterator (_Base::_find (_item)); }
        typename _Base::const_iterator find (const _TpItem& _item) const { return typename _Base::const_iterator (_Base::_find (_item)); }
    };
  }
}

#endif // __CGTL__CGT_BASE_LIST_H_
