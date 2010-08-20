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
 * \file cgt/base/iterator/iterator_ptr.h
 * \brief Contains definition of an iterator for general use that has a pointer to the iterated item.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-01 01:01:04 +0100 (Tue, 01 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 90 $
 */

#ifndef __CGTL__CGT_BASE_ITERATOR_ITERATOR_PTR_H_
#define __CGTL__CGT_BASE_ITERATOR_ITERATOR_PTR_H_

#include "iterator_type.h"


namespace cgt
{
  namespace base
  {
    namespace iterator
    {
      /*
       * \class _IteratorPtr
       * \brief A base class for iterators that hold pointers to the item.
       * \author Leandro Costa
       * \date 2009
       *
       * This is just the definition of the template.
       */

      template<typename _TpItem, template<typename> class _TpIterator>
        class _IteratorPtr;


      /*
       * \class _IteratorPtr<_TpItem, _TpCommon>
       * \brief A base class for iterators that hold pointers to the item.
       * \author Leandro Costa
       * \date 2009
       *
       * This is a specialization of a base class for common iterators that
       * hold pointers to the item.
       */

      template<typename _TpItem>
        class _IteratorPtr<_TpItem, _TpCommon>
        {
          private:
            friend class _IteratorPtr<_TpItem, _TpConst>;

          private:
            typedef _IteratorPtr<_TpItem, _TpCommon>  _Self;
            typedef _IteratorPtr<_TpItem, _TpConst>   _SelfConst;

          protected:
            _IteratorPtr () : _ptr (NULL) { }
            _IteratorPtr (_TpItem* _p) : _ptr (_p) { }
            virtual ~_IteratorPtr () { }

          public:
            virtual const bool operator==(const _Self& _other) const { return (_ptr == _other._ptr); }
            virtual const bool operator==(const _SelfConst& _other) const { return (_ptr == _other._ptr); }
            virtual const bool operator!=(const _Self& _other) const { return !(*this == _other); }
            virtual const bool operator!=(const _SelfConst& _other) const { return !(*this == _other); }

          protected:
            virtual void _incr () = 0;

          protected:
            _TpItem* _ptr;
        };


      /*
       * \class _IteratorPtr<_TpItem, _TpConst>
       * \brief A base class for iterators that hold pointers to the item.
       * \author Leandro Costa
       * \date 2009
       *
       * This is a specialization of a base class for const iterators that
       * hold pointers to the item.
       */

      template<typename _TpItem>
        class _IteratorPtr<_TpItem, _TpConst>
        {
          private:
            friend class _IteratorPtr<_TpItem, _TpCommon>;

          private:
            typedef _IteratorPtr<_TpItem, _TpConst>   _Self;
            typedef _IteratorPtr<_TpItem, _TpCommon>  _SelfCommon;

          protected:
            _IteratorPtr () : _ptr (NULL) { }
            _IteratorPtr (_TpItem* _p) : _ptr (_p) { }
            _IteratorPtr (const _SelfCommon& _it) : _ptr (_it._ptr) { }
            virtual ~_IteratorPtr () { }

          public:
            virtual const bool operator==(const _Self& _other) const { return (_ptr == _other._ptr); }
            virtual const bool operator==(const _SelfCommon& _other) const { return (_ptr == _other._ptr); }
            virtual const bool operator!=(const _Self& _other) const { return !(*this == _other); }
            virtual const bool operator!=(const _SelfCommon& _other) const { return !(*this == _other); }

          protected:
            virtual void _incr () = 0;

          protected:
            _TpItem* _ptr;
        };
    }
  }
}

#endif // __CGTL__CGT_BASE_ITERATOR_ITERATOR_PTR_H_
