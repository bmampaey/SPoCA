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
 * \file cgt/base/exception/mem_except.h
 * \brief Contains definitions of exceptions related to memory operations
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_BASE_EXCEPTION_MEM_EXCEPT_H_
#define __CGTL__CGT_BASE_EXCEPTION_MEM_EXCEPT_H_

#include "exception.h"

namespace cgt
{
  namespace base
  {
    namespace exception
    {
      /*!
       * \class mem_except
       * \brief Exception thrown by allocator where it's not possible to allocate memory.
       * \author Leandro Costa
       * \date 2009
       *
       * \exception mem_except It was not possible to allocate memory.
       */
      class mem_except : public exception
      {
        public:
          mem_except (const char* _m) : exception (_m) { }
      };
    }
  }
}

#endif
