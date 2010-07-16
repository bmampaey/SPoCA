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
 * \file cgt/vertex_iterator.h
 * \brief Contains definition of a vertex iterator.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-11-24 01:55:40 +0100 (Tue, 24 Nov 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 81 $
 */

#ifndef __CGTL__CGT_VERTEX_ITERATOR_H_
#define __CGTL__CGT_VERTEX_ITERATOR_H_

#include "base/list_iterator.h"


namespace cgt
{
  /*!
   * \class _VertexIterator
   * \brief The _VertexIterator class template.
   * \author Leandro Costa
   * \date 2009
   *
   * A \b _VertexIterator is an iterator of vertices. It is used when
   * we are interested only in the vertices of the graph, and not in
   * the adjacency list of each vertex.
   */

  template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
    class _VertexIterator : public cgt::base::_ListIterator<_GraphNode<_TpVertex, _TpEdge>, _TpIterator>
    {
      private:
        typedef _VertexIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpCommon> _SelfCommon;

      private:
        typedef _GraphNode<_TpVertex, _TpEdge>                _Node;
        typedef cgt::base::_ListIterator<_Node, _TpIterator>  _Base;
        typedef _GraphVertex<_TpVertex>                       _Vertex;
        typedef typename _TpIterator<_Vertex>::reference      reference;

      public:
        _VertexIterator () { }
        _VertexIterator (const _Base &_iterator) : _Base (_iterator) { }
        _VertexIterator (const _SelfCommon& _it) : _Base (_it) { }

      public:
        reference operator*() const;
    };


  template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator>
    typename _TpIterator<_GraphVertex<_TpVertex> >::reference _VertexIterator<_TpVertex, _TpEdge, _TpIterator>::operator*() const
    {
      return _Base::operator*().vertex ();
    }
}

#endif // __CGTL__CGT_VERTEX_ITERATOR_H_
