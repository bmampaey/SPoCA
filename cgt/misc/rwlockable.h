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
 * \file cgt/misc/rwlockable.h
 * \brief Defines a read/write lockable interface.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-22 23:43:47 -0200 (Dom, 22 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 79 $
 */

#ifndef __CGTL__CGT_MISC_RWLOCKABLE_H_
#define __CGTL__CGT_MISC_RWLOCKABLE_H_

#include <pthread.h>


namespace cgt
{
  /*!
   * \namespace cgt::misc
   * \brief Where are defined all auxiliary types, classes, files, etc.
   * \author Leandro Costa
   * \date 2009
   */

  namespace misc
  {
    /*!
     * \class _RWLockable
     * \brief A read/write lockable interface.
     * \author Leandro Costa
     * \date 2009
     *
     * A class that implements mutual exclusion using pthread read-write lock.
     */
    class _RWLockable
    {
      protected:
        _RWLockable () { pthread_rwlock_init(&_rwlock, NULL); }
        virtual ~_RWLockable () { pthread_rwlock_destroy(&_rwlock); }

      public:
        void rlock () const { pthread_rwlock_rdlock (&_rwlock); }
        void wlock () const { pthread_rwlock_wrlock (&_rwlock); }
        void unlock () const { pthread_rwlock_unlock (&_rwlock); }

      private:
        mutable pthread_rwlock_t  _rwlock;
    };
  }
}

#endif // __CGTL__CGT_MISC_RWLOCKABLE_H_
