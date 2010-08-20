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
 * \file cgt/base/list_iterator.h
 * \brief Contains definition of list iterator (common and const).
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_LIST_ITERATOR_H_
#define __CGTL__CGT_BASE_LIST_ITERATOR_H_

#include "iterator/iterator_ptr.h"


namespace cgt
{
  namespace base
  {
    /*!
     * \class _ListIterator
     * \brief An iterator for list container.
     * \author Leandro Costa
     * \date 2009
     *
     * This is the default iterator for list container.
     */

    template<typename _TpItem, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
      class _ListIterator : public cgt::base::iterator::_IteratorPtr<_ListItemBase<_TpItem>, _TpIterator>
    {
      private:
        typedef cgt::base::iterator::_IteratorPtr<_ListItemBase<_TpItem>, _TpIterator> _Base;

      private:
        typedef _ListIterator<_TpItem, _TpIterator>                     _Self;
        typedef _ListIterator<_TpItem, cgt::base::iterator::_TpCommon>  _SelfCommon;
        typedef _ListItem<_TpItem>                                      _Item;

      private:
        typedef typename _TpIterator<_TpItem>::pointer    pointer;
        typedef typename _TpIterator<_TpItem>::reference  reference;

      private:
        using _Base::_ptr;

      public:
        _ListIterator () { }
        _ListIterator (_Item* _p) : _Base (_p) { }
        _ListIterator (const _SelfCommon& _it) : _Base (_it) { }
        virtual ~_ListIterator () { }

      private:
        void _incr () { _ptr = _ptr->_next; }

      public:
        reference operator*() const;
        pointer operator->() const;

        _Self& operator++();
        const _Self operator++(int);
    };


    template<typename _TpItem, template<typename> class _TpIterator>
      typename _TpIterator<_TpItem>::reference _ListIterator<_TpItem, _TpIterator>::operator*() const
      {
        return static_cast<_Item *>(_ptr)->_data;
      }

    template<typename _TpItem, template<typename> class _TpIterator>
      typename _TpIterator<_TpItem>::pointer _ListIterator<_TpItem, _TpIterator>::operator->() const
      {
        return &(operator*());
      }

    template<typename _TpItem, template<typename> class _TpIterator>
      _ListIterator<_TpItem, _TpIterator>& _ListIterator<_TpItem, _TpIterator>::operator++()
      {
        _incr ();
        return *this;
      }

    template<typename _TpItem, template<typename> class _TpIterator>
      const _ListIterator<_TpItem, _TpIterator> _ListIterator<_TpItem, _TpIterator>::operator++(int)
      {
        _Self _it = *this;
        _incr ();
        return _it;
      }

    template<typename _TpItem, typename _Predicate>
      _ListIterator<_TpItem> find_if (_ListIterator<_TpItem> _it, _ListIterator<_TpItem> _end, _Predicate _pred)
      {
        while (_it != _end && ! _pred (*_it))
          ++_it;

        return _it;
      }

    template<typename _TpItem, typename _Predicate, typename _Parm>
      _ListIterator<_TpItem> find_if (_ListIterator<_TpItem> _it, _ListIterator<_TpItem> _end, _Predicate _pred, const _Parm& _parm)
      {
        while (_it != _end && ! _pred (*_it, _parm))
          ++_it;

        return _it;
      }
  }
}

#endif // __CGTL__CGT_BASE_LIST_ITERATOR_H_
