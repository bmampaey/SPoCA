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
 * \file cgt/graph_vertex.h
 * \brief Contains definition of a vertex.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-11 01:05:43 +0100 (Fri, 11 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 100 $
 */

#ifndef __CGTL__CGT_GRAPH_VERTEX_H_
#define __CGTL__CGT_GRAPH_VERTEX_H_


namespace cgt
{
  /*!
   * \class _GraphVertex
   * \brief A wrapper for vertex, contains only the attribute \b _value of type \b _TpVertex.
   * \author Leandro Costa
   * \date 2009
   *
   * A \b _GraphVertex is a structure that encapsulates the vertex. Inside,
   * it has only the object vertex. When a vertex is inserted in the graph,
   * an object of this type is created as an inner attribute of structure
   * \b _GraphNode.
   *
   * \code
   *  _______________________
   * |  _GraphVertex's size  |
   * |_______________________|
   * |   sizeof(_TpVertex)   |
   * |_______________________|
   *
   * \endcode
   */

  template<typename _TpVertex>
    class _GraphVertex
    {
      public:
        _GraphVertex (const _TpVertex &_v) { _value = _v; }

      public:
        const bool operator==(const _GraphVertex<_TpVertex> &_v) const { return _value == _v.value (); }
        const bool operator!=(const _GraphVertex<_TpVertex> &_v) const { return ! (*this == _v); }

        const bool operator==(const _TpVertex &_v) const { return _value == _v; }
        const bool operator!=(const _TpVertex &_v) const { return ! (*this == _v); }

      public:
        const _TpVertex&  value () const { return _value; }
        _TpVertex&  value () { return _value; }

      private:
        _TpVertex _value;
    };
}

#endif // __CGTL__CGT_GRAPH_VERTEX_H_
