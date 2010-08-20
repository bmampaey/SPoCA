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
 * \file cgt/base/alloc/storage.h
 * \brief Contains definitions of storage and chunk, used by allocator.
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_BASE_ALLOC_STORAGE_H_
#define __CGTL__CGT_BASE_ALLOC_STORAGE_H_

#include <iostream>

#include "../exception/mem_except.h"
#include "../../misc/safe_wlock.h"
#include "../../misc/cxxtest_defs.h"

UT_CXXTEST_DEFINE_CLASS(storage_cxx);


namespace cgt
{
  namespace base
  {
    namespace alloc
    {
      const static size_t _CHUNK_SIZE = 0xFFFF; /* 64K-1 */

      /*!
       * \class _Storage
       * \brief A storage for a chunk-based allocator, encapsulates \b new and \b delete.
       * \author Leandro Costa
       * \date 2009
       *
       * This is the storage used by a chunk-based allocator, implementation based on the
       * example found in <b>The C++ Programming Language, 3rd Edition, by Bjarne Stroustrup, page 570</b>.
       */

      template<typename _TpItem, size_t _ChunkSize = _CHUNK_SIZE /* 64K-1 */>
        class _Storage : private cgt::misc::_RWLockable
      {
        private:
          UT_CXXTEST_FRIEND_CLASS(::storage_cxx);

        private:
          friend class cgt::misc::_Safe_WLock;

        private:
          /*!
           * \class _Chunk
           * \brief The chunk structure for a chunk-based allocator, default size = 64K-1 bytes.
           * \author Leandro Costa
           * \date 2009
           *
           * This is the storage's chunk structure used by a chunk-based allocator, implementation based on the
           * example found in <b>The C++ Programming Language, 3rd Edition, by Bjarne Stroustrup, page 570</b>.
           */

          class _Chunk
          {
            public:
              /*!
               * \struct _Block
               * \brief The block structure for a chunk-based allocator, used to identify a free block.
               * \author Leandro Costa
               * \date 2009
               */

              struct _Block
              {
                _Block* _next; /** < points to the next free block (valid only if this block is free) */
              };

            public:
              _Chunk () : _next (NULL) { _init (); }

            private:
              void _init ()
              {
                size_t _blocksize = sizeof (_TpItem) >= sizeof (_Block*) ? sizeof (_TpItem):sizeof (_Block*);
                unsigned int _numblocks = _ChunkSize/_blocksize;
                char* _ptr_last = &(_block [(_numblocks - 1) * _blocksize]);

                for (char* _ptr = _block; _ptr < _ptr_last; _ptr += _blocksize)
                {
                  reinterpret_cast<_Block *>(_ptr)->_next = reinterpret_cast<_Block *>(_ptr + _blocksize);
                }

                reinterpret_cast<_Block *>(_ptr_last)->_next = NULL;
              }

            public:
              char _block [_ChunkSize]; /** < a block of the chunk */
              _Chunk* _next; /** < points to the next allocated chunk */
          };

        private:
          typedef typename _Chunk::_Block _Block;

        public:
          _Storage () : _head (NULL), _free (NULL) { }
          virtual ~_Storage () { _destroy (); }

        private:
          void _add_chunk ();
          void _destroy ();

        public:
          _TpItem* allocate (); /** < a thread-safe allocator method */
          void deallocate (_TpItem* _ptr); /** < a thread-safe deallocator method */

        private:
          _Chunk* _head; /** < the first allocated chunk of the storage */
          _Block* _free; /** < points to the first free block of any chunk */
      };


      template<typename _TpItem, size_t _ChunkSize>
        void _Storage<_TpItem, _ChunkSize>::_add_chunk ()
        {
          _Chunk* _ptr = new _Chunk ();

          if (! _ptr)
            throw cgt::base::exception::mem_except ("Not available memory");

          _ptr->_next = _head;
          _head = _ptr;
          _free = reinterpret_cast<_Block *>(_ptr->_block);
        }

      template<typename _TpItem, size_t _ChunkSize>
        void _Storage<_TpItem, _ChunkSize>::_destroy ()
        {
          _Chunk* _ptr = NULL;

          while (_head)
          {
            _ptr = _head;
            _head = _head->_next;
            delete _ptr;
          }
        }

      template<typename _TpItem, size_t _ChunkSize>
        _TpItem* _Storage<_TpItem, _ChunkSize>::allocate ()
        {
          cgt::misc::_Safe_WLock (*this); /** < guarantees thread-safety */

          if (! _free)
            _add_chunk ();

          _TpItem* _ptr = reinterpret_cast<_TpItem *>(_free);
          _free = _free->_next;

          return _ptr;
        }

      template<typename _TpItem, size_t _ChunkSize>
        void _Storage<_TpItem, _ChunkSize>::deallocate (_TpItem* _ptr)
        {
          cgt::misc::_Safe_WLock (*this); /** < guarantees thread-safety */

          reinterpret_cast<_Block *>(_ptr)->_next = _free;
          _free = reinterpret_cast<_Block *>(_ptr);
        }
    }
  }
}

#endif // __CGTL__CGT_BASE_ALLOC_STORAGE_H_
