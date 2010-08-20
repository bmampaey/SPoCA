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
 * \file cgt/base/alloc/allocator.h
 * \brief Contains definition of a chunk-based allocator.
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_BASE_ALLOC_ALLOCATOR_H_
#define __CGTL__CGT_BASE_ALLOC_ALLOCATOR_H_

#include "storage.h"
#include <stdlib.h>
#include <malloc.h>


namespace cgt
{
  namespace base
  {
    /*!
     * \namespace cgt::base::alloc
     * \brief Where are defined the default chunk-based allocator and structures related.
     * \author Leandro Costa
     * \date 2009
     */

    namespace alloc
    {
      /*!
       * \class _Allocator
       * \brief A chunk-based allocator.
       * \author Leandro Costa
       * \date 2009
       * \todo Implement thread-safety for allocator.
       *
       * A chunk-based allocator, implementation based on the example found in
       * <b>The C++ Programming Language, 3rd Edition, by Bjarne Stroustrup, page 570</b>.
       */

      template<typename _TpItem>
        class _Allocator
        {
          public:
            typedef size_t          size_type;
            typedef ptrdiff_t       difference_type;
            typedef _TpItem*        pointer;
            typedef const _TpItem*  const_pointer;
            typedef _TpItem&        reference;
            typedef const _TpItem&  const_reference;
            typedef _TpItem         value_type;

            /*!
             * \struct rebind
             * \brief A classical rebind struct for the allocator.
             * \author Leandro Costa
             * \date 2009
             */

            template <class _U> struct rebind { typedef _Allocator<_U> other; };

            _Allocator() { };
            _Allocator(const _Allocator&) { };
            template <class _U> _Allocator(const _Allocator<_U>&) { };
            ~_Allocator() { };

            pointer address(reference x) const { return &x; }
            const_pointer address(const_reference x) const { return &x; }

            pointer allocate(size_type size, _Allocator<_TpItem>::const_pointer hint = 0) { return static_cast<pointer>(_storage.allocate ()); }
            void deallocate(pointer p, size_type n) { _storage.deallocate (p); }
            size_type max_size() const { return (size_t (-1) / sizeof (_TpItem)); }
            void construct(pointer p, const _TpItem& val) { new (static_cast<_TpItem *>(p)) _TpItem (val); }
            void destroy(pointer p) { p->~_TpItem (); }

          private:
            static _Storage<_TpItem> _storage; /** < each allocator has its own \i static storage, that's why we need to guarantee mutual exclusion */
        };

      template<typename _TpItem>
        _Storage<_TpItem> _Allocator<_TpItem>::_storage;
    }
  }
}

#endif // __CGTL__CGT_BASE_ALLOC_ALLOCATOR_H_
