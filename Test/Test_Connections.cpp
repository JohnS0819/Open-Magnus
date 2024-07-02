#include "pch.h"
#include "gtest/gtest.h"
#include "Connections.hpp"
#include <random>
#include "Testing_Utility_Functions.hpp"
#include <queue>
void deepCopyDfs(int position, std::vector<Graph::Collision_Leaf>& graph, int& index, ConnectionManager::compressedNode* output, std::unordered_map<int, int>& map, std::vector<int>& available);

std::vector<Graph::Collision_Leaf> UTILITY::generate_nodes(int n);


template<typename graphtype1, typename graphtype2>
bool areGraphsIsomorphic(std::vector<graphtype1>& G1, std::vector<graphtype2>& G2) {
	if (G1.size() != G2.size()) {
		return false;
	}

	auto search = [&](int j) {
		auto coord = G1[j].coord;
		for (int i = 0; i < G1.size(); ++i) {
			if (G2[i].coord == coord) {
				return i;
			}
		}
		return -1;
	};
	for (int i = 0; i < G1.size(); ++i) {
		int j = search(i);
		if (j == -1) {
			return false;
		}
		for (int k = 1; k < 7; ++k) {
			if (G2[j].children[k] == -1 || G1[i].children[k] == -1) {
				if (G2[j].children[k] != G1[i].children[k]) {
					return false;
				}
				continue;
			}
			int i_neighbor = G1[i].children[k];
			int j_neighbor = G2[j].children[k];

			if (G2[j_neighbor].coord != G1[i_neighbor].coord) {
				return false;
			}

		}
	}
	return true;
}

#define graphSize 1000

TEST(CONNECTION_TESTS, Test_Deep_Copy) {
	std::vector<Graph::Collision_Leaf>G1 = UTILITY::generate_nodes(graphSize);
	std::vector<Graph::Collision_Leaf>G2 = UTILITY::generate_nodes(graphSize);
	std::vector<Graph::Collision_Leaf>G3 = UTILITY::generate_nodes(graphSize);
	for (int i = 0; i < graphSize; ++i) {
		G2[i].coord = G2[i].coord + Geometry::grid_coordinate{3 * graphSize, 0};
		G3[i].coord = G3[i].coord + Geometry::grid_coordinate{ 0, 3 * graphSize };
	}

	std::vector<Graph::Collision_Leaf> G;

	for (int i = 0; i < graphSize; ++i) {
		G.push_back(G1[i]);
		
		for (int j = 1; j < 7; ++j) {
			if (G1[i].children[j] == -1) continue;
			G[3 * i].children[j] *= 3;
		}
		G.push_back(G2[i]);
		for (int j = 1; j < 7; ++j) {
			if (G2[i].children[j] == -1) continue;
			G[3 * i + 1].children[j] = 1 + G2[i].children[j] * 3;
		}


		G.push_back(G3[i]);
		for (int j = 1; j < 7; ++j) {
			if (G3[i].children[j] == -1) continue;
			G[3 * i + 2].children[j] = 2 + G3[i].children[j] * 3;
		}

	}

	std::vector< ConnectionManager::compressedNode> output(3 * graphSize);
	std::vector<int> baka;
	std::unordered_map<int, int> map;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution dis(0, graphSize - 1);

	map[0] = 0;
	int index = 0;

	deepCopyDfs(0, G, index, output.data(), map, baka);
	++index;
	map[1] = index;
	deepCopyDfs(1, G, index, output.data(), map, baka);
	++index;
	map[2] = index;
	deepCopyDfs(2, G, index, output.data(), map, baka);


	EXPECT_TRUE(areGraphsIsomorphic(G,output));




}
#define NUM_OF_MOLECULES 4000
#define NUM_OF_TILES 4000
#define CHANCE_OF_CONNECTOR 0.1
#define CHANCE_OF_OCCUPIED 0.5
#define MAX_NUM_OF_TILE_SLOTS 6
#define MAX_SUBGRAPH_SIZE 100
#define CHANCE_OF_NEW_SUBGRAPH 0.8


class MockConnectionManager_RandomMolecules : public MockFixture, public ::testing::Test{
public:

	using TileManager::instance;

	MockConnectionManager_RandomMolecules(){
		
		//StealInstance();
	}
	std::vector<int> number_of_submolecules;
	std::vector<bool> connected;

	






	virtual void SetUp() {
		number_of_submolecules.resize(NUM_OF_MOLECULES);
		connected.resize(NUM_OF_MOLECULES, false);
		std::fill(number_of_submolecules.begin(), number_of_submolecules.end(), 1);

		moleculeOwners.resize(NUM_OF_MOLECULES, -1);


		TESTING_MOLECULEMANGAER_OVERRIDE::setNumber(NUM_OF_MOLECULES);
		tiles_.resize(NUM_OF_TILES);
		partitionSizes.resize(NUM_OF_MOLECULES);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution subgraph_size_dist(1, MAX_SUBGRAPH_SIZE);
		std::uniform_int_distribution subgraph_dist(0, NUM_OF_MOLECULES);
		std::uniform_real_distribution<double> subgraph_chance_dist(0, 1);
		std::uniform_int_distribution molecule_dist(0, NUM_OF_MOLECULES - 1);
		std::uniform_real_distribution<double> bool_dist(0, 1);
		std::uniform_int_distribution slot_dist(1, MAX_NUM_OF_TILE_SLOTS);
		/*for (auto& i : number_of_submolecules) {
			i = subgraph_dist(gen);
		}*/
		for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
			partitionSizes[i].resize(number_of_submolecules[i]);
			for (auto& j : partitionSizes[i]) {
				j = subgraph_size_dist(gen);
			}
		}
		for (auto& i : this->tiles_) {
			auto chance = bool_dist(gen);
			if (chance < CHANCE_OF_CONNECTOR) {
				i.type = GridTile::connecter;
			}
			else {
				i.type = GridTile::disconnector;
			}
			i.spots.resize(1 + slot_dist(gen), { { -1,-1 },-1,-1 });

			int parent;
			int subparent;
			bool occupied;
			if (bool_dist(gen) < CHANCE_OF_OCCUPIED) {
				parent = molecule_dist(gen);
				subparent = molecule_dist(gen) % number_of_submolecules[parent];
				occupied = true;
			}
			else {
				occupied = false;
				parent = -1;
				subparent = -1;
			}
			i.spots[0].owner = parent;
			i.spots[0].subowner = subparent;
			for (int j = 1; j < i.spots.size(); ++j) {
				auto occ_chance = bool_dist(gen);
				if (occ_chance < CHANCE_OF_OCCUPIED) {
					if (i.type == GridTile::disconnector && subgraph_chance_dist(gen) < CHANCE_OF_NEW_SUBGRAPH && occupied) {
						partitionSizes[parent].push_back(subgraph_size_dist(gen));
						i.spots[j].owner = parent;
						i.spots[j].subowner = number_of_submolecules[parent];
						++number_of_submolecules[parent];
					}
					else {
						int mol = molecule_dist(gen);
						int subowner = subgraph_dist(gen) % number_of_submolecules[mol];
						while (mol == parent && i.type == GridTile::connecter && subowner != subparent) {
							mol = molecule_dist(gen);
							subowner = subgraph_dist(gen) % number_of_submolecules[mol];
						}
						if (parent != -1 && i.type == GridTile::connecter && (parent != mol || subparent != subowner)) {
							connected[parent] = true;
							connected[mol] = true;
						}


						i.spots[j].owner = mol;
						i.spots[j].subowner = subowner;
					}
				}
			}
		}
		generateConnectivityMap();
		generatePartitions();
		generateTasks();

		buffer_.resize(tasks.size());
		partitionGroupLastPositions.resize(tasks.size(),0);
		offsets_.resize(NUM_OF_MOLECULES);
		for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
			offsets_[i].resize(number_of_submolecules[i],-1); 
		}


		for (int i = 0; i < tasks.size(); ++i) {
			configureTask(i);
		}
		StealInstance();
	}

	virtual void TearDown() {
		
	}



};

TEST_F(MockConnectionManager_RandomMolecules, Test_Offsets_Methods) {
	for (int i = 0; i < tasks.size(); ++i) {
		std::unordered_set<int> mols;
		for (auto& part : tasks[i]) {
			for (auto& mol : partitions[part]) {
				mols.insert(mol.first);
			}
		}
		for (auto& mol : mols) {
			setoffsets(mol, i);
		}
		setBufferSize(i);
		std::unordered_map<int, std::pair<int, int>> positions;
		std::unordered_map<int, int> counts;
		for (auto& mol : mols) {
			for (int j = 0; j < offsets_[mol].size(); ++j) {
				if (offsets_[mol][j] == -1) continue;
				positions[offsets_[mol][j]] = { mol,j };
				++counts[offsets_[mol][j]];
			}


		}
		for (auto& count : counts) {
			EXPECT_EQ(count.second, 1);
		}
		for (auto& pos : positions) {
			for (int j = 1; j < partitionSizes[pos.second.first][pos.second.second]; ++j) {
				EXPECT_TRUE(positions.find(pos.first + j) == positions.end());
			}
		}
	}
}

#define MIN_RANGE 0
#define MAX_RANGE 1000
#define MAX_MOLECULE_SIZE 200
#define EDGE_DENSITY 0.01
#define CHANCE_OF_DEFECT 0.1





class MechanismMoleculeConnectionFixture: public MockFixture, public testing::Test {
public:
	std::unordered_set<Geometry::grid_coordinate> TileOccupied;
	std::unordered_set<Geometry::grid_coordinate> MolOccupied;
	std::vector<Molecule> Molecules;
	std::vector<Graph::Geometry_Graph> graphs;
	std::vector<Geometry::Coordinate_Transform> Transforms;
	std::vector<std::vector<int>> defectMaps;

	void checkHierarchyIntergrity(Molecule& M) {
		auto& nodes = AccessNodes(M);
		auto& coordMap = AccessCoordMap(M);
		auto& available = TESTING_MOLECULEMANGAER_OVERRIDE::getAvailable(M);

		Geometry::grid_coordinate deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
		int inverse[] = { 3,4,5,0,1,2 };

		EXPECT_EQ(coordMap.back().size(), 1);
		EXPECT_EQ(available.size(), coordMap.size());
		EXPECT_EQ(nodes.size(), coordMap.size());
		
		for (int i = 1; i < nodes.size(); ++i) {
			std::unordered_set<int> unused;
			for (auto& j : coordMap[i]) {
				int pos = j.second;
				auto& node = nodes[i][pos];
				EXPECT_EQ(node.coord, j.first);
				std::unordered_map<int, int> children;
				for (int k = 0; k < 7; ++k) {
					if (node.children[k] == -1) continue;
					++children[node.children[k]];
					int child = node.children[k];
					auto& child_node = nodes[i - 1][child];
					EXPECT_TRUE(coordMap[i - 1].find(child_node.coord) != coordMap[i-1].end());
					EXPECT_EQ(coordMap[i - 1][child_node.coord], child);
					EXPECT_EQ(Graph::Collision_Hierarchy::findParent(child_node.coord).first, node.coord);
					EXPECT_EQ(Graph::Collision_Hierarchy::findParent(child_node.coord).second, k);
				}
				EXPECT_GE(children.size(), 1);
			}
			for (auto& j : coordMap[i - 1]) {
				auto& coord = j.first;
				auto& index = j.second;
				auto parent = Graph::Collision_Hierarchy::findParent(coord);
				auto parent_coord = parent.first;
				auto parent_subindex = parent.second;
				EXPECT_TRUE(coordMap[i].find(parent_coord) != coordMap[i].end());
				int parent_index = coordMap[i][parent_coord];
				EXPECT_EQ(nodes[i][parent_index].coord, parent_coord);
				if (nodes[i][parent_index].children[parent_subindex] != index) {
					std::cerr << i << '\t' << parent_index << '\t' << index << '\t' << parent_subindex << std::endl;
				}
				EXPECT_EQ(nodes[i][parent_index].children[parent_subindex], index);
			}
			for (auto& j : available[i]) {
				EXPECT_TRUE(unused.find(j) == unused.end());
				unused.insert(j);
			}
			for (auto& j : coordMap[i]) {
				EXPECT_TRUE(unused.find(j.second) == unused.end());
			}

		}

		


	}

	void verifyBaseLevelMoleculeIntegrity(Molecule& M) {
		auto& nodes = AccessNodes(M)[0];
		auto& coordMap = AccessCoordMap(M)[0];
		auto& available = TESTING_MOLECULEMANGAER_OVERRIDE::getAvailable(M)[0];
		Geometry::grid_coordinate deltas[] = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
		int inverse[] = { 3,4,5,0,1,2 };

		std::unordered_set<int> unoccupied;
		std::unordered_map<int, int> occupied;
		for (auto& i : available) {
			EXPECT_TRUE(unoccupied.find(i) == unoccupied.end());
			unoccupied.insert(i);
		}
		EXPECT_EQ(available.size() + coordMap.size(), nodes.size());

		for (auto& i : coordMap) {
			EXPECT_EQ(nodes[i.second].coord, i.first);
			EXPECT_TRUE(unoccupied.find(i.second) == unoccupied.end()) << nodes[i.second].children[0] << '\t' << available.size() << std::endl
				<< nodes[i.second].children[1] << std::endl
				<< nodes[i.second].children[2] << std::endl
				<< nodes[i.second].children[3] << std::endl
				<< nodes[i.second].children[4] << std::endl
				<< nodes[i.second].children[5] << std::endl
				<< nodes[i.second].children[6] << std::endl;

				
			int index = i.second;
			for (int k = 1; k < 7; ++k) {
				if (nodes[index].children[k] != -1) {
					int child = nodes[index].children[k];
					auto expectedCoord = nodes[index].coord + deltas[k - 1];
					EXPECT_EQ(nodes[child].coord, expectedCoord);
					EXPECT_EQ(nodes[child].children[1 + inverse[k - 1]], index);
				}
				else if (coordMap.find(nodes[index].coord + deltas[k - 1]) != coordMap.end()) {
					int other = coordMap[nodes[index].coord + deltas[k - 1]];
					EXPECT_EQ(nodes[other].children[1 + inverse[k - 1]], -1);
					for (int k2 = 1; k < 7; ++k) {
						EXPECT_TRUE(nodes[other].children[k2] != index);
					}
				}

			}
			++occupied[i.second];
		}
		for (auto& i : occupied) {
			EXPECT_EQ(i.second, 1);
		}

		for (auto& i : coordMap) {
			int index = i.second;
			std::unordered_map<int, int> children_counts;
			for (int k = 1; k < 7; ++k) {
				if (nodes[index].children[k] == -1) continue;
				++children_counts[nodes[index].children[k]];
			}
			for (auto& j : children_counts) {
				EXPECT_EQ(j.second, 1);
				EXPECT_TRUE(unoccupied.find(j.first) == unoccupied.end());
			}
		}


	}



	virtual void SetUp() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution X_dist(MIN_RANGE, MAX_RANGE);
		std::uniform_int_distribution Y_dist(MIN_RANGE, MAX_RANGE);
		std::uniform_int_distribution Slot_Dist(0, MAX_NUM_OF_TILE_SLOTS);
		std::uniform_real_distribution<double> bool_dist(0, 1);


		std::vector<Geometry::grid_coordinate> deltas = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };

		for (int i = 0; i < NUM_OF_TILES; ++i) {
			Geometry::grid_coordinate initial = { X_dist(gen),Y_dist(gen) };
			if (TileOccupied.find(initial) != TileOccupied.end()) continue;
			Geometry::coordinate casted = initial;
			TileOccupied.insert(initial);
			tiles_.push_back({ casted });
			auto& tile = tiles_.back();
			//tile.spots.resize(0);
			tile.spots.push_back({});
			tile.spots[0].location = initial;
			tile.spots[0].owner = -1;
			tile.spots[0].subowner = -1;
			int num_of_slots = Slot_Dist(gen);
			if (bool_dist(gen) < CHANCE_OF_CONNECTOR) {
				tile.type = GridTile::connecter;
			}
			else {
				tile.type = GridTile::disconnector;
			}
			std::shuffle(deltas.begin(), deltas.end(), gen);
			for (int j = 0; j < num_of_slots; ++j) {
				if (TileOccupied.find(initial + deltas[j]) != TileOccupied.end()) continue;
				tile.spots.push_back({ });
				tile.spots.back().location = initial + deltas[j];
				tile.spots.back().owner = -1;
				tile.spots.back().subowner = -1;
				TileOccupied.insert(initial + deltas[j]);
			}
			if (tile.spots.size() == 1) {
				tiles_.pop_back();
			}
		}

		std::uniform_int_distribution mol_dist(1, MAX_MOLECULE_SIZE);
		std::uniform_int_distribution rotation_dist(0, 5);
		std::uniform_int_distribution translation_dist(MIN_RANGE, MAX_RANGE);

		for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
			int size = mol_dist(gen);
			Geometry::Coordinate_Transform T;
			int rot = rotation_dist(gen);
			for (int j = 0; j < rot; ++j) {
				T.Rotate();
			}
			T.translate({ translation_dist(gen),translation_dist(gen) });
			auto potential = generate_Graph_in_Boundary(MIN_RANGE, MAX_RANGE, size, MolOccupied, T, EDGE_DENSITY);
			if (potential.nodes.size() == 0) continue;
			for (auto& j : potential.nodes) {
				MolOccupied.insert(T(j.coord));
			}
			graphs.push_back(potential);

			Transforms.push_back(T);
			Graph::Collision_Hierarchy G(std::move(potential));
			while (G.getNodes().back().size() > 1) {
				G.subdivide();
			}
			Molecules.push_back({ std::move(G), T });


		}

		//make the nodes array inside the collider contain empty slots, and populate the available array with those positions
		for (auto& M : Molecules) {
			auto& nodes = AccessNodes(M);
			auto& coord_map = AccessCoordMap(M);
			auto& available = TESTING_MOLECULEMANGAER_OVERRIDE::getAvailable(M);
			available.resize(nodes.size());

			auto& basenodes = nodes[0];
			auto& baseCoordMap = coord_map[0];
			std::vector<int> replacementMap(basenodes.size(), 0);
			int offset_ = 0;
			int N_ = basenodes.size();
			int relative_iterator = 0;
			std::unordered_set<int> unused;
			for (int y = 0; y < N_; ++y, ++relative_iterator) {
				while (bool_dist(gen) < CHANCE_OF_DEFECT) {
					++offset_;
					Graph::Collision_Leaf empty;
					basenodes.insert(basenodes.begin() + relative_iterator, empty);
					unused.insert(relative_iterator);
					available[0].push_back(relative_iterator);
					for (int v = 0; v < 7; ++v) {
						if (bool_dist(gen) < 0.5) {
							basenodes[relative_iterator].children[v] = X_dist(gen);
						}
						else {
							basenodes[relative_iterator].children[v] = -1;
						}
					}
					basenodes[relative_iterator].coord.i = X_dist(gen);
					basenodes[relative_iterator].coord.j = Y_dist(gen);
					++relative_iterator;
				}
				replacementMap[y] = y + offset_;
			}
			for (int u = 0; u < basenodes.size(); ++u) {
				if (unused.find(u) != unused.end()) continue;
				for (int p = 1; p < 7; ++p) {
					if (basenodes[u].children[p] == -1) continue;
					basenodes[u].children[p] = replacementMap[basenodes[u].children[p]];
				}
			}
			for (auto& o : baseCoordMap) {
				o.second = replacementMap[o.second];
			}
			if(nodes.size() > 1) {
				for (auto& o : nodes[1]) {
					for (int c = 0; c < 7; ++c) {
						if (o.children[c] != -1) {
							o.children[c] = replacementMap[o.children[c]];
						}
					}
				}
			}
			defectMaps.push_back(replacementMap);
		}

		moleculeOwners.resize(Molecules.size(), -1);
		TESTING_MOLECULEMANGAER_OVERRIDE::setNumber(Molecules.size());
		partitionSizes.resize(Molecules.size());
		moleculeTransforms.resize(Molecules.size());
		for (int i = 0; i < Molecules.size(); ++i) {
			moleculeTransforms[i] = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(Molecules[i]);
		}
		kdTree.build();
		StealInstance();
		for (int i = 0; i < Molecules.size(); ++i) {
			instance().query(Molecules[i], i);
		}
		for (int i = 0; i < Molecules.size(); ++i) {
			Molecules[i].callbackConnections(i);
		}


		instance().generateConnectivityMap();
		instance().generatePartitions();
		instance().generateTasks();

		for (int i = 0; i < ACCESSOR.gettasks(instance()).size(); ++i) {
			instance().configureTask(i);
		}
	}


};


TEST_F(MechanismMoleculeConnectionFixture, Test_Compression_And_Decompression) {
	const auto& connectivityMap_ = ACCESSOR.getconnectivityMap(instance());
	const auto& tasks_ = ACCESSOR.gettasks(instance());
	auto& partitionsSizes_ = ACCESSOR.getpartitionSizes(instance());
	auto& partitions_ = ACCESSOR.getpartitions(instance());
	auto& moleculeOwners_ = ACCESSOR.getmoleculeOwners(instance());
	auto& partitionLinks_ = ACCESSOR.getpartitionLinks(instance());
	auto& TrivialTasks_ = ACCESSOR.getTrivialTasks(instance());
	
	std::unordered_map<Geometry::grid_coordinate, int> counts;
	for (auto& i : Molecules) {
		auto &coordMap_ = AccessCoordMap(i)[0];
		auto& nodes_ = AccessNodes(i)[0];
		auto& T = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(i);
		auto& available = TESTING_MOLECULEMANGAER_OVERRIDE::getAvailable(i)[0];
		std::unordered_set<int> unoccupied;
		for (auto& i : available) {
			EXPECT_TRUE(unoccupied.find(i) == unoccupied.end());
			unoccupied.insert(i);
		}
		for (auto& j : coordMap_) {
			EXPECT_TRUE(unoccupied.find(j.second) == unoccupied.end());
			EXPECT_EQ(j.first, nodes_[j.second].coord);
			++counts[T(j.first)];
		}
	}
	for (auto& i : counts) {
		EXPECT_EQ(i.second, 1);
	}




	std::vector<Molecule*> new_stuff;
	std::vector<Molecule*> pointers;
	std::vector<Molecule*> used;
	for (auto& i : Molecules) {
		verifyBaseLevelMoleculeIntegrity(i);
		checkHierarchyIntergrity(i);
	}


	std::unordered_map<Geometry::grid_coordinate, int> originalCoords;
	std::unordered_map<int, std::unordered_set<Geometry::grid_coordinate>> originalCoordMaps;
	//for (auto& part: partitions_) {
	//	for (auto& component : part) {
	//		int parent = component.first;
	//		if (CheckedMolecule.find(parent) != CheckedMolecule.end()) continue;
	//		CheckedMolecule.insert(parent);
	//		auto& mol = Molecules[parent];
	//		auto& coords = AccessCoordMap(mol)[0];
	//		auto& transform = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(mol);
	//		for (auto& i : coords) {
	//			EXPECT_TRUE(originalCoords.find(transform(i.first)) == originalCoords.end());
	//			originalCoords[transform(i.first)] = parent;
	//		}
	//	}
	//}
	for (auto& j : TrivialTasks_) {
		auto& mol = Molecules[j];
		auto& coords = AccessCoordMap(mol)[0];
		auto& transform = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(mol);
		EXPECT_TRUE(originalCoordMaps.find(j) == originalCoordMaps.end());
		auto& output = originalCoordMaps[j];
		for (auto& i : coords) {
			EXPECT_TRUE(originalCoords.find(transform(i.first)) == originalCoords.end());
			EXPECT_TRUE(output.find(transform(i.first)) == output.end());
			output.insert(transform(i.first));
			originalCoords[transform(i.first)] = j;
		}
	}




	for (int i = 0; i < Molecules.size(); ++i) {
		pointers.push_back(&Molecules[i]);
	}

	for (int i = 0; i < partitionLinks_.size(); ++i) {
		if (partitionLinks_[i] == -1) {
			partitionLinks_[i] = pointers.size();
			new_stuff.push_back({ new Molecule });
			pointers.push_back(new_stuff.back());
			used.push_back(pointers.back());
		}
		else {
			used.push_back(pointers[partitionLinks_[i]]);
		}
	}

	for (int i = 0; i < TrivialTasks_.size(); ++i) {
		TileManager::instance().configureTrivialTask(i);
	}


	for (auto& i : TrivialTasks_) {
		int biggest = partitionsSizes_[i].back();
		std::unordered_set<Geometry::grid_coordinate> newCoords;
		for (int j = 0; j < partitionsSizes_[i].size() - 1; ++j) {
			if (j == biggest) {
				ConnectionManager::instance().trim(Molecules[i], i, j);
				used.push_back({ &Molecules[i] });
			}
			else {
				new_stuff.push_back({ new Molecule(ConnectionManager::instance().extractDirectly(Molecules[i],i,j)) });
				used.push_back(new_stuff.back());
				pointers.push_back(new_stuff.back());
			}
			auto& output = *used.back();
			verifyBaseLevelMoleculeIntegrity(output);
			checkHierarchyIntergrity(output);
			auto& coordMap = AccessCoordMap(output)[0];
			auto& transform = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(output);
			for (auto& k : coordMap) {
				if (newCoords.find(transform(k.first)) != newCoords.end()) {
					std::cerr << i << '\t' << j << std::endl;
				}
				EXPECT_TRUE(newCoords.find(transform(k.first)) == newCoords.end());
				newCoords.insert(transform(k.first));
				if (originalCoords.find(transform(k.first)) == originalCoords.end()) {
					std::cerr << i << '\t' << j << std::endl;
				}
				EXPECT_TRUE(originalCoords.find(transform(k.first)) != originalCoords.end());
			}
		}
		EXPECT_EQ(newCoords, originalCoordMaps[i]);
 		if (newCoords != originalCoordMaps[i]) {
			std::cerr << i << std::endl;
		}
	}





	std::unordered_map<int, std::unordered_set<Geometry::grid_coordinate>> correct_coords;
	for (int i = 0; i < partitionLinks_.size(); ++i) {
		//if (partitionLinks_[i] == -1) continue;
		int owner = i;
		EXPECT_TRUE(correct_coords.find(owner) == correct_coords.end());
		auto& output = correct_coords[owner];
		for (auto& j : partitions_[i]) {
			std::unordered_set<int> visited;
			auto& nodes_ = AccessNodes(Molecules[j.first])[0];
			auto& coordMap_ = AccessCoordMap(Molecules[j.first])[0];
			auto& T = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(Molecules[j.first]);
			int begin = TESTING_MOLECULEMANGAER_OVERRIDE::getComponentMap(Molecules[j.first])[j.second];
			std::queue<int> queue;
			queue.push(begin);
			while (!queue.empty()) {
				int current = queue.front();
				queue.pop();
				if (visited.find(current) != visited.end()) continue;
				visited.insert(current);
				EXPECT_TRUE(coordMap_.find(nodes_[current].coord) != coordMap_.end());
				for (int k = 1; k < 7; ++k) {
					if (nodes_[current].children[k] == -1) continue;
					queue.push(nodes_[current].children[k]);
				}
				EXPECT_TRUE(output.find(T(nodes_[current].coord)) == output.end());
				output.insert(T(nodes_[current].coord));
			}
		}
	}



	/*std::vector<int> orphans;
	for (int i = 0; i < moleculeOwners_.size(); ++i) {
		if (moleculeOwners_[i] == -1) {
			orphans.push_back(i);
		}
	}
	for (int i = 0; i < partitionLinks_.size(); ++i) {
		if (partitionLinks_[i] != -1) continue;
		if (!orphans.empty()) {
			partitionLinks_[i] = orphans.back();
			orphans.pop_back();
		}
		else {
			partitionLinks_[i] = Molecules.size();
			Molecules.push_back({});
		}

	}*/

	



	buffer_.resize(tasks_.size());
	partitionGroupLastPositions.resize(tasks_.size(), 0);
	offsets_.resize(Molecules.size());
	
	

	for (int i = 0; i < Molecules.size(); ++i) {
		offsets_[i].resize(partitionsSizes_[i].size(), -1);
	}
	for (int i = 0; i < tasks_.size(); ++i) {
		std::unordered_set<int> mols;
		for (auto& part : tasks_[i]) {
			for (auto& mol : partitions_[part]) {
				mols.insert(mol.first);
			}
		}
		for (auto& mol : mols) {
			setoffsets(mol, i);
		}
		setBufferSize(i);
		for (auto& mol : mols) {
			extractAndCompress(Molecules[mol], mol);
		}
	}



	//verify that every cannibalized molecule has the correct nodes
	for (int i = 0; i < partitionLinks_.size(); ++i) {

			
		update(i, *pointers[partitionLinks_[i]]);
		//std::cout << "\n\n\n" << std::endl;
		EXPECT_TRUE(correct_coords.find(i) != correct_coords.end());
		auto& T = TESTING_MOLECULEMANGAER_OVERRIDE::getTransform(*pointers[partitionLinks_[i]]);
		auto& coordMap_ = AccessCoordMap(*pointers[partitionLinks_[i]])[0];
		for (auto& j : coordMap_) {
			EXPECT_TRUE(correct_coords[i].find(T(j.first)) != correct_coords[i].end());
		}
		for (auto& j : correct_coords[i]) {
			EXPECT_TRUE(coordMap_.find(T.inverse()(j)) != coordMap_.end());
		}
		verifyBaseLevelMoleculeIntegrity(*pointers[partitionLinks_[i]]);
		checkHierarchyIntergrity(*pointers[partitionLinks_[i]]);

	}

	for (auto i : new_stuff) {
		delete(i);
	}


}