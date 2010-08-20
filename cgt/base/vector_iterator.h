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
 * \file cgt/base/vector_iterator.h
 * \brief Contains definition of vector iterator (common and const).
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-02 00:05:33 +0100 (Wed, 02 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 96 $
 */

#ifndef __CGTL__CGT_BASE_VECTOR_ITERATOR_H_
#define __CGTL__CGT_BASE_VECTOR_ITERATOR_H_

#include "iterator/iterator_ptr.h"


namespace cgt
{
  namespace base
  {
    /*!
     * \class _VectorIterator
     * \brief An iterator for vector container.
     * \author Leandro Costa
     * \date 2009
     *
     * This is the default iterator for vector container.
     */

    template<typename _TpItem, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
      class _VectorIterator : public cgt::base::iterator::_IteratorPtr<_TpItem*, _TpIterator>
    {
      private:
        typedef cgt::base::iterator::_IteratorPtr<_TpItem*, _TpIterator>  _Base;
        typedef _VectorIterator<_TpItem, _TpIterator>                     _Self;
        typedef _VectorIterator<_TpItem, cgt::base::iterator::_TpCommon>  _SelfCommon;

      private:
        typedef typename _TpIterator<_TpItem>::pointer    pointer;
        typedef typename _TpIterator<_TpItem>::reference  reference;

      private:
        using _Base::_ptr;

      public:
        _VectorIterator () { }
        _VectorIterator (_TpItem** _p) : _Base (_p) { }
        _VectorIterator (const _SelfCommon& _it) : _Base (_it) { }
        virtual ~_VectorIterator () { }

      private:
        void _incr () { _ptr++; }

      public:
        const bool operator<(const _Self& _other) const { return (_ptr < _other._ptr); }
        const bool operator>(const _Self& _other) const { return (_ptr > _other._ptr); }
        const bool operator>=(const _Self& _other) const { return (_ptr >= _other._ptr); }
        reference operator*() const { return **_ptr; }
        pointer operator->() const { return *_ptr; }

        _Self& operator++() { _incr (); return *this; }
        const _Self operator++(int) { _Self _it = *this; _incr (); return _it; }
        const size_t operator-(const _Self &_other) const { return _ptr - _other._ptr; }
    };


    template<typename _TpItem, typename _Predicate>
      _VectorIterator<_TpItem> find_if (_VectorIterator<_TpItem> _it, _VectorIterator<_TpItem> _end, _Predicate _pred)
      {
        while (_it != _end && ! _pred (*_it))
          ++_it;

        return _it;
      }

    template<typename _TpItem, typename _Predicate, typename _Parm>
      _VectorIterator<_TpItem> find_if (_VectorIterator<_TpItem> _it, _VectorIterator<_TpItem> _end, _Predicate _pred, const _Parm& _parm)
      {
        while (_it != _end && ! _pred (*_it, _parm))
          ++_it;

        return _it;
      }
  }
}

#endif // __CGTL__CGT_BASE_VECTOR_ITERATOR_H_
