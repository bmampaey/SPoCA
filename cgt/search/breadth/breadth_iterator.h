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
 * \file cgt/search/breadth/breadth_iterator.h
 * \brief Contains definition of an iterator that implements breadth-first search algorithm.
 * \author Leandro Costa
 * \date 2009
 *
 * $LastChangedDate: 2009-12-01 01:01:04 +0100 (Tue, 01 Dec 2009) $
 * $LastChangedBy: leandro.costa $
 * $Revision: 90 $
 */

#ifndef __CGTL__CGT_SEARCH_BREADTH_BREADTH_ITERATOR_H_
#define __CGTL__CGT_SEARCH_BREADTH_BREADTH_ITERATOR_H_

#include "../search_iterator.h"
#include "../../base/queue.h"


namespace cgt
{
  namespace search
  {
    /*!
     * \namespace cgt::search::breadth
     * \brief Where are defined structures related to breadth-first search algorithms.
     * \author Leandro Costa
     * \date 2009
     */

    namespace breadth
    {
      /*!
       * \class _BreadthIterator
       * \brief An iterator that implements breadth-first search by nodes.
       * \author Leandro Costa
       * \date 2009
       *
       * breadth-first search algorithm
       *  - _it_node: iterator pointing to the begining of the node list;
       *  - _it_node_end: iterator pointing to the end of the node list.
       *
       * initialize:
       *  - paint the first node with GRAY;
       *  - put it on the queue;
       *  - turn it the current node;
       *  - paint all the others with WHITE.
       *
       * operator++:
       *  - visit the adjacency list of the queue's front node:
       *    - if a WHITE node is found:
       *      - paint it with GRAY, put it on the queue and turn it the current node.
       *    - if there is no more WHITE nodes:
       *      - pop the queue's front node, paint it with BLACK;
       *      - continue visiting the adjacency list of the new queue's front node
       *  - if the queue is empty:
       *    - use _it_node to find the next WHITE node.
       *    - if a WHITE node is found:
       *      - paint it with GRAY, put it on the queue and turn it the current node.
       *    - if no more WIHTE nodes are found:
       *      - point the current node to NULL.
       */

      template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator = cgt::base::iterator::_TpCommon>
        class _BreadthIterator : public _SearchIterator<_TpVertex, _TpEdge, cgt::base::queue, _TpIterator>
      {
        public:
          typedef _SearchInfo<_TpVertex, _TpEdge> _BreadthInfo;

        private:
          typedef _SearchIterator<_TpVertex, _TpEdge, cgt::base::queue, _TpIterator>    _Base;
          typedef _BreadthIterator<_TpVertex, _TpEdge, _TpIterator>                     _Self;
          typedef _BreadthIterator<_TpVertex, _TpEdge, cgt::base::iterator::_TpCommon>  _SelfCommon;
          typedef _GraphNode<_TpVertex, _TpEdge>                                        _Node;
          typedef typename cgt::base::list<_Node>::iterator                             _NodeIterator;
          typedef _SearchState<_TpVertex, _TpEdge>                                      _BreadthState;

        private:
          using _Base::_ptr_node;
          using _Base::_it_node;
          using _Base::_it_node_end;
          using _Base::_stContainer;
          using _Base::_global_time;

        public:
          _BreadthIterator () { }
          _BreadthIterator (_Node* const _ptr_n) : _Base (_ptr_n) { }
          _BreadthIterator (_Node* const _ptr_n, const _NodeIterator& _it_begin, const _NodeIterator& _it_end) : _Base (_ptr_n, _it_begin, _it_end) { }
          _BreadthIterator (const _NodeIterator& _it, const _NodeIterator& _it_begin, const _NodeIterator& _it_end) : _Base (&(*_it), _it_begin, _it_end) { }
          _BreadthIterator (const _SelfCommon& _it) : _Base (_it) { }

        public:
          _Self& operator++();

        public:
          const _BreadthInfo* const info (const _Node* const _ptr_node) { return _get_depth_info_by_node (*_ptr_node); }
          const _BreadthInfo* const info (const _Node& _node) { return _get_depth_info_by_node (_node); }

          typename cgt::base::list<_BreadthInfo>::iterator info_begin () { return _Base::_infoList.begin (); }
          typename cgt::base::list<_BreadthInfo>::iterator info_end () { return _Base::_infoList.end (); }
      };

      template<typename _TpVertex, typename _TpEdge, template<typename> class _TpIterator>
        _BreadthIterator<_TpVertex, _TpEdge, _TpIterator>& _BreadthIterator<_TpVertex, _TpEdge, _TpIterator>::operator++()
        {
          /*
           * visit the adjacency list of the queue's front node:
           *  - if a WHITE node is found:
           *    - paint it with GRAY, put it on the queue and turn it the current node.
           *  - if there is no more WHITE nodes:
           *    - dequeue the queue's front node, paint it with BLACK;
           *    - continue visiting the adjacency list of the new queue's front node
           *  if the queue is empty:
           *  - use _it_node to find the next WHITE node.
           *  - if a WHITE node is found:
           *    - paint it with GRAY, put it on the queue and turn it the current node.
           *  - if no more WIHTE nodes are found:
           *    - point the current node to NULL.
           */

          _ptr_node = NULL;

          while (! _stContainer.empty ())
          {
            _BreadthState *_ptr_state  = _stContainer.first ();

            while (! _ptr_state->adj_finished ())
            {
              if (_has_color (_ptr_state->_adj_node (), _BreadthInfo::WHITE))
              {
                _ptr_node = &(_ptr_state->_adj_node ());
                _ptr_state->adj_incr ();
                _stContainer.enqueue (_BreadthState (*_ptr_node));
                _discover_node (*_ptr_node, &(_ptr_state->node ()), ++_global_time);
                break;
              }
              else
                _ptr_state->adj_incr ();
            }

            if (! _ptr_node)
            {
              _BreadthState *_ptr = _stContainer.dequeue ();
              _finish_node (_ptr->node (), ++_global_time);
              delete _ptr;
            }
            else
              break;
          }

          if (! _ptr_node)
          {
            while (_it_node != _it_node_end && ! _has_color (*_it_node, _BreadthInfo::WHITE))
              ++_it_node;

            if (_it_node != _it_node_end)
            {
              _ptr_node = &(*_it_node);
              _stContainer.enqueue (_BreadthState (*_it_node));
              _discover_node (*_it_node, NULL, ++_global_time);
            }
          }

          return *this;
        }
    }
  }
}

#endif // __CGTL__CGT_SEARCH_BREADTH_BREADTH_ITERATOR_H_
