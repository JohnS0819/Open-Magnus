#include "pch.h"
#include "gtest/gtest.h"
#include "Graph.hpp"
#include "Molecule.hpp"


//TEST(GRAPH_TESTS, query_tests) {
//	Graph::Geometry_Graph G;
//	G.nodes.resize(9);
//	auto& nodes = G.nodes;
//	for (int i = 0; i < 9; ++i) {
//		G.nodes[i].children[0] = 1;
//	}
//	for (int i = 1; i < 8; ++i) {
//		G.nodes[i].children[1] = i - 1;
//		G.nodes[i].children[6] = i + 1;
//	}
//	for (int i = 0; i < 9; ++i) {
//		G.nodes[i].coord = { 0,i };
//	}
//	G.nodes[0].children[6] = 1;
//	G.nodes[8].children[1] = 7;
//
//	
//	std::vector<Graph::Collision_Leaf> output(9);
//	std::vector<int> baka;
//	int index = 0;
//	std::unordered_map<int, int> map;
//	map[0] = 0;
//	deepCopyDfs(0, G.nodes, index, output.data(), map, baka);
//
//	EXPECT_TRUE(areGraphsIsomorphic(output, G.nodes));
//}