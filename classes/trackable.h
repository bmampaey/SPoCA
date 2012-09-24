#pragma once
#ifndef trackable_H
#define trackable_H


#include <vector>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>

#include <map>
#include <set>

#include "tools.h"
#include "constants.h"
#include "ColorMap.h"
#include "Region.h"
#include "gradient.h"

extern std::string filenamePrefix;
extern ColorType newColor;

class RegionGraph {
public:
	class node;
	class edge;

	class const_iterator {
		const RegionGraph * graph;
		std::map<Region*,node>::const_iterator it;
		
	public:
		const_iterator(const RegionGraph* graph, std::map<Region*,node>::const_iterator it) : graph(graph), it(it) {}

		const node& operator*() { return it->second; }
		void operator++() { ++it; }
		void operator++(int) { operator++(); }
		bool operator==(const const_iterator& other) { return it == other.it; }
		bool operator!=(const const_iterator& other) { return !(operator==(other)); }
		const node* operator->() { return &(it->second); }
	};

	class iterator {
		RegionGraph * graph;
		std::map<Region*,node>::iterator it;
	public:	
		iterator(RegionGraph* graph, std::map<Region*,node>::iterator it) : graph(graph), it(it) {}

		node& operator*() { return it->second; }
		void operator++() { ++it; }
		void operator++(int) { operator++(); }
		bool operator==(const iterator& other) { return it == other.it; }
		bool operator!=(const iterator& other) { return !(operator==(other)); }
		node* operator->() { return &(it->second); }
	};

	class edge {
	public:
		node* from;
		node* to;
		int weight;
	
		edge(RegionGraph::node* from, RegionGraph::node* to, int weight) : from(from), to(to), weight(weight) {}

		bool operator==(const edge& other) const { return from == other.from && to == other.to && weight == other.weight; }
	};


	class node {
		const RegionGraph* graph;
		Region* region;
		std::vector<edge> in_edges;
		std::vector<edge> out_edges;

		class edge_cmp {
		public:
      			bool operator()(const edge& a, const edge& b) { return a.weight < b.weight; }
		};
	public:
		typedef std::vector<edge>::const_iterator const_iterator;
		
		node() {}
		node(const RegionGraph * graph, Region* region) : graph(graph), region(region) {}
			
		const node* biggestParent() const {
			return std::max_element(in_edges.begin(), in_edges.end(), edge_cmp())->from;
		}

		const node* biggestSon() const {
			return std::max_element(out_edges.begin(), out_edges.end(), edge_cmp())->to;
		}
		
		Region* get_region() const {
			return region;
		}

		bool path(const node* to, std::set<node*>* visited);

		bool path(const node* to) {
			std::set<node*> visited;

			return path(to, &visited);
		}

		void colorize();

		const_iterator in_begin() const { return in_edges.begin(); }
		const_iterator in_end() const { return in_edges.end(); }
		const_iterator out_begin() const { return out_edges.begin(); }
		const_iterator out_end() const { return out_edges.end(); }

		friend RegionGraph;
	};

private:
	std::map<Region*, node> nodes;

public:
	void add_edge(node* from, node* to, int weight) {
		from->out_edges.push_back(edge(from, to, weight));
		to->in_edges.push_back(edge(from, to, weight));
	}

	node* get_node(Region* region) {
		return &nodes[region];
	}

	void add_node(Region* region) {
		nodes[region] = node(this, region);
	}

	const_iterator begin() const { return const_iterator(this, nodes.begin()); }
	const_iterator end() const { return const_iterator(this, nodes.end()); }
	iterator begin() { return iterator(this, nodes.begin()); }
	iterator end() { return iterator(this, nodes.end()); }
};



//Ordonate the images according to time
void ordonate(std::vector<ColorMap*>& images);

// Return a vector of indices of the images vector ordonated according to time
std::vector<unsigned> imageOrder(const std::vector<ColorMap*>& images);

// Compute the number of pixels common to 2 regions from 2 images, with derotation
unsigned overlay_derotate(ColorMap* image1, const Region* region1, ColorMap* image2, const Region* region2);

// Compute the number of pixels common to 2 regions from 2 images
unsigned overlay(ColorMap* image1, const Region* region1, ColorMap* image2, const Region* region2);

// Output a graph in the dot format
void ouputGraph(const RegionGraph& g, const std::vector<std::vector<Region*> >& regions, const std::string graphName, bool isColored = true);

// Output regions in the region format
void ouputRegions(const std::vector<std::vector<Region*> >& regions, std::string filename);

void recolorFromRegions(ColorMap* image, const std::vector<Region*>& regions);
#endif
