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
 * \file cgt/graph_node.h
 * \brief Contains definition of a node.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-10 03:20:24 +0100 (Thu, 10 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 99 $
 */

#ifndef __CGTL__CGT_GRAPH_NODE_H_
#define __CGTL__CGT_GRAPH_NODE_H_

#include "graph_vertex.h"
#include "graph_adjlist.h"

namespace cgt
{
  /*!
   * \class _GraphNode
   * \brief It contains the vertex, its adjacency list and an inverted ajacency list to ease graph's invertion.
   * \author Leandro Costa
   * \date 2009
   *
   * A \b _GraphNode is a structure of the graph that contains a vertex and an
   * adjacency list. Each adjacency to its vertex contains a reference to the
   * other node of the relationship. So, from the node of a vertex \b v it's
   * possible to iterate by its adjacency list and visit all vertices reachable
   * from \b v.
   *
   * A _GraphNode has a vertex and two adjacency lists. The overhead of a vertex
   * is 0, and the size of an adjacency list is <b>12 + 16n bytes</b>, where n is
   * the number of edges of the vertex. So, the _GraphNode's size is
   * <b>24 + 32n + sizeof (_TpVertex) bytes</b>.
   */

  template<typename _TpVertex, typename _TpEdge>
    class _GraphNode
    {
      private:
        typedef _GraphNode<_TpVertex, _TpEdge>    _Self;
        typedef _GraphVertex<_TpVertex>           _Vertex;
        typedef _GraphAdjList<_TpVertex, _TpEdge> _AdjList;

      public:
        _GraphNode (const _TpVertex &_v) : _vertex (_v) { };

      public:
        _Vertex& vertex ()  { return _vertex; }
        const _Vertex& vertex () const  { return _vertex; }
        _AdjList&  adjlist () { return _adjList; }
        const _AdjList&  adjlist () const { return _adjList; }
        _AdjList&  iadjlist () { return _invAdjList; }
        const _AdjList&  iadjlist () const { return _invAdjList; }
        _TpVertex& value () { return _vertex.value (); }
        const _TpVertex& value () const { return _vertex.value (); }

      public:
        _GraphEdge<_TpVertex, _TpEdge>* _get_edge (const _Vertex& _v) const;

      public:
        void _insert (_GraphEdge<_TpVertex, _TpEdge>& _e, _Self& _n);
        void _insert_inverse (_GraphEdge<_TpVertex, _TpEdge>& _e, _Self& _n);
        void _invert_edges ();

      private:
        _Vertex   _vertex;
        _AdjList  _adjList;

        /*
         * This is a list of inverted edges: if a node n1 has an
         * inverted edge pointing to n2, it means n2 has an edge
         * pointing to n1. This way we can invert a graph in O(V),
         * just swaping the two lists: _adjList and _invAdjList.
         */

        _AdjList  _invAdjList;
    };

  template<typename _TpVertex, typename _TpEdge>
    _GraphEdge<_TpVertex, _TpEdge>* _GraphNode<_TpVertex, _TpEdge>::_get_edge (const _Vertex& _v) const
    {
      return _adjList._get_edge (_v);
    }

  template<typename _TpVertex, typename _TpEdge>
    void _GraphNode<_TpVertex, _TpEdge>::_insert (_GraphEdge<_TpVertex, _TpEdge>& _e, _Self& _n)
    {
      _adjList._insert (_e, _n);
      _n._insert_inverse (_e, *this);
    }

  template<typename _TpVertex, typename _TpEdge>
    void _GraphNode<_TpVertex, _TpEdge>::_insert_inverse (_GraphEdge<_TpVertex, _TpEdge>& _e, _Self& _n)
    {
      _invAdjList._insert (_e, _n);
    }

  /*
   * time complexity: O(1)
   */
  template<typename _TpVertex, typename _TpEdge>
    void _GraphNode<_TpVertex, _TpEdge>::_invert_edges ()
    {
      _AdjList::swap (_adjList, _invAdjList);
    }
}

#endif // __CGTL__CGT_GRAPH_NODE_H_
