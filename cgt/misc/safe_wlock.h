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
 * \file cgt/misc/safe_wlock.h
 * \brief Defines a safe lockable operation for write.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-22 23:43:47 -0200 (Dom, 22 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 79 $
 */

#ifndef __CGTL__CGT_MISC_SAFE_WLOCK_H_
#define __CGTL__CGT_MISC_SAFE_WLOCK_H_

#include <pthread.h>
#include "rwlockable.h"


namespace cgt
{
  namespace misc
  {
    /*!
     * \class _Safe_WLock
     * \brief A safe lockable operation for write.
     * \author Leandro Costa
     * \date 2009
     *
     * A class that implements safe lockable operation for write. The constructor
     * receives a reference to a read/write lockable object. When a \b _Safe_WLock is
     * created the lockable object is locked. When the \b _Safe_WLock is destructed
     * the lockable object is unlocked. The right way to use it is creating the safe
     * lock n the stack in the begining of a scope as follows:
     *
     * \code
     * void C::func ()
     * {
     *   // class C inherits from _RWLockable
     *   _Safe_WLock lock (*this); // here this object is locked
     *   ... // do what you need
     *   // at the end lock is destructed, and this object is unlocked
     * }
     * \endcode
     */

    class _Safe_WLock
    {
      public:
        _Safe_WLock (_RWLockable& _l) : _lockable (_l) { _lockable.wlock (); }
        virtual ~_Safe_WLock () { _lockable.unlock (); }

      private:
        _RWLockable& _lockable;
    };
  }
}

#endif
