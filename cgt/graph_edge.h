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
 * \file cgt/graph_edge.h
 * \brief Contains definition of an edge.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-11 01:05:43 +0100 (Fri, 11 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 100 $
 */

#ifndef __CGTL__CGT_GRAPH_EDGE_H_
#define __CGTL__CGT_GRAPH_EDGE_H_


namespace cgt
{
  /*!
   * \class _GraphEdge
   * \brief A wrapper for edge, contains \b _value of type \b _TpEdge and references to vertices: \b _vertex1 and \b _vertex2.
   * \author Leandro Costa
   * \date 2009
   *
   * A \b _GraphEdge is a structure that encapsulates the edge. Inside, it has
   * the object edge and the references for the vertices linked by the edge.
   * When an edge is inserted in the graph, an object of this type is created
   * and pushed in the edge's list of the structure \b _GraphAdjMatrix.
   *
   * The _GraphEdge's size has an overhead of 2 references to vertices. In 32-bits
   * architectures it means we need 8 more bytes to represent each edge in our graph.
   *
   * \code
   *  _______________________
   * |   _GraphEdge's size   |
   * |_______________________|
   * |  8 + sizeof(_TpEdge)  |
   * |_______________________|
   *
   * \endcode
   */

  template<typename _TpVertex, typename _TpEdge>
    class _GraphEdge
    {
      private:
        typedef _GraphVertex<_TpVertex> _Vertex;

      public:
        _GraphEdge (const _TpEdge& _e, _Vertex& _v1, _Vertex& _v2) : _value (_e), _vertex1 (_v1), _vertex2 (_v2) { }

      public:
        const _TpEdge& value () const { return _value; }
        _TpEdge& value () { return _value; }
        _Vertex& v1 () const { return _vertex1; }
        _Vertex& v2 () const { return _vertex2; }

      private:
        _TpEdge   _value;
        _Vertex&  _vertex1;
        _Vertex&  _vertex2;
    };
}

#endif // __CGTL__CGT_GRAPH_EDGE_H_
