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
 * \file cgt/base/hash_iterator_base.h
 * \brief Contains definition of hash iterator base (common and const).
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate$
 * $LastChangedBy$
 * $Revision$
 */

#ifndef __CGTL__CGT_BASE_HASH_ITERATOR_BASE_H_
#define __CGTL__CGT_BASE_HASH_ITERATOR_BASE_H_

#include "hash_item.h"
#include "pair.h"
#include "iterator/iterator_ptr.h"


namespace cgt
{
  namespace base
  {
    template<typename _TpKey, typename _TpItem, typename _Alloc>
      class hash;

    /*!
     * \class _HashIteratorBase
     * \brief The base for the hash iterator.
     * \author Leandro Costa
     * \date 2009
     *
     * This is the base for the hash iterator. It defines two different
     * implementations, one for const and other for common iterators.
     * This way the hash iterator doesn't need to worry about the copy
     * constructor of a const iterator receiving a common iterator as argument.
     */

    template<typename _TpKey, typename _TpItem, typename _Alloc, template<typename> class _TpIterator>
      class _HashIteratorBase;

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      class _HashIteratorBase<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpCommon> : public cgt::base::iterator::_IteratorPtr<_HashItem<pair<const _TpKey, _TpItem> >, cgt::base::iterator::_TpCommon>
    {
      private:
        friend class _HashIteratorBase<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpConst>;

      private:
        typedef _HashItem<pair<const _TpKey, _TpItem> >                                     _Item;
        typedef cgt::base::iterator::_IteratorPtr<_Item, cgt::base::iterator::_TpCommon>    _Base;
        typedef hash<_TpKey, _TpItem, _Alloc>                                               _Hash;

      public:
        _HashIteratorBase () : _Base (NULL), _ptr_hash (NULL) { }
        _HashIteratorBase (_Item* _p, _Hash* _p_hsh) : _Base (_p), _ptr_hash (_p_hsh) { }
        virtual ~_HashIteratorBase () { }

      protected:
        _Hash* _ptr_hash; /** < pointer to the hash, shouldn't be destructed */
    };

    template<typename _TpKey, typename _TpItem, typename _Alloc>
      class _HashIteratorBase<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpConst> : public cgt::base::iterator::_IteratorPtr<_HashItem<pair<const _TpKey, _TpItem> >, cgt::base::iterator::_TpConst>
    {
      private:
        typedef _HashItem<pair<const _TpKey, _TpItem> >                                     _Item;
        typedef cgt::base::iterator::_IteratorPtr<_Item, cgt::base::iterator::_TpConst>     _Base;
        typedef _HashIteratorBase<_TpKey, _TpItem, _Alloc, cgt::base::iterator::_TpCommon>  _SelfCommon;
        typedef hash<_TpKey, _TpItem, _Alloc>                                               _Hash;

      public:
        _HashIteratorBase () : _Base (NULL), _ptr_hash (NULL) { }
        _HashIteratorBase (_Item* _p, _Hash* _p_hsh) : _Base (_p), _ptr_hash (_p_hsh) { }
        _HashIteratorBase (const _SelfCommon& _it) : _Base (_it), _ptr_hash (_it._ptr_hash) { }
        virtual ~_HashIteratorBase () { }

      protected:
        _Hash* _ptr_hash; /** < pointer to the hash, shouldn't be destructed */
    };
  }
}

#endif // __CGTL__CGT_BASE_HASH_ITERATOR_BASE_H_
