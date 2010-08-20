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
 * \file cgt/base/exception/exception.h
 * \brief Contains the base exception class definition.
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_BASE_EXCEPTION_EXCEPTION_H_
#define __CGTL__CGT_BASE_EXCEPTION_EXCEPTION_H_

#include <string.h>


namespace cgt
{
  namespace base
  {
    /*!
     * \namespace cgt::base::exception
     * \brief The namespace where are defined all exceptions.
     * \author Leandro Costa
     * \date 2009
     */

    namespace exception
    {
      /*!
       * \class exception
       * \brief The base exception class, which all exceptions are directly or indirectly derived from.
       * \author Leandro Costa
       * \date 2009
       */

      class exception
      {
        public:
          exception (const char* _m) { _message = strdup (_m); }

        public:
          const char* message () { return _message; }

        private:
          char *_message;
      };
    }
  }
}

#endif
