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
 * \file cgt/graph_type.h
 * \brief Contains the types used to define directed and undirected graphs.
 * \author Leandro Costa
 * \date 2009
 */

#ifndef __CGTL__CGT_GRAPH_TYPE_H_
#define __CGTL__CGT_GRAPH_TYPE_H_


namespace cgt
{
  /*!
   * \struct _GraphTypeDirected
   * \typedef _Directed
   * \brief Used to define a directed graph.
   * \author Leandro Costa
   * \date 2009
   */

  typedef struct _GraphTypeDirected
  {
    enum { _directed = true };
  } _Directed;

  /*!
   * \struct _GraphTypeUndirected
   * \typedef _Undirected
   * \brief Used to define an undirected graph.
   * \author Leandro Costa
   * \date 2009
   */

  typedef struct _GraphTypeUndirected
  {
    enum { _directed = false };
  } _Undirected;
}

#endif
