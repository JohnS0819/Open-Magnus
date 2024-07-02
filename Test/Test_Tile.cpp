#include "pch.h"
#include "gtest/gtest.h"
#include "Tile.hpp"
#include <random>
#include <queue>
#include <array>
#include "Hungarian.hpp"
#include "Testing_Utility_Functions.hpp"

//parameters for general mechanism tests
#define NUM_OF_MOLECULES 2000
#define NUM_OF_TILES 2000
#define CHANCE_OF_CONNECTOR 0.3
#define CHANCE_OF_OCCUPIED 0.5
#define MAX_NUM_OF_TILE_SLOTS 6
#define MAX_SUBGRAPH_SIZE 100
#define CHANCE_OF_NEW_SUBGRAPH 0.8

//parameters for combined mechanism molecule tests
#define MIN_RANGE 0
#define MAX_RANGE 1000
#define MAX_MOLECULE_SIZE 100
#define EDGE_DENSITY 0.15
#define CHANCE_OF_DEFECT 0.1



class MockTileManager : public TileManager , public ::testing::Test{
public:
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



	}

	virtual void TearDown() {

	}




	
};

typedef std::unordered_map<std::pair<int, int>, std::vector<std::pair<int, int>>> AdjacencyMap;

// Depth-First Search (DFS) function to find connected components
void DFS(const AdjacencyMap& adj_map, const std::pair<int, int>& node, std::unordered_set<std::pair<int, int>>& visited, std::vector<std::pair<int, int>>& component) {
	visited.insert(node); // Mark the current node as visited
	component.push_back(node);

	// Visit all adjacent nodes
	for (const std::pair<int, int>& neighbor : adj_map.at(node)) {
		if (visited.find(neighbor) == visited.end()) { // If the neighbor is not visited
			DFS(adj_map, neighbor, visited, component);
		}
	}
}

// Function to find connected components in an undirected graph
std::vector<std::vector<std::pair<int, int>>> FindConnectedComponents(const AdjacencyMap& adj_map) {
	std::unordered_set<std::pair<int, int>> visited;
	std::vector<std::vector<std::pair<int, int>>> components;

	for (const auto& entry : adj_map) {
		const std::pair<int, int>& node = entry.first;
		if (visited.find(node) == visited.end()) { // If the node is not visited
			std::vector<std::pair<int, int>> component;
			DFS(adj_map, node, visited, component);
			components.push_back(component);
		}
	}

	return components;
}


TEST_F(MockTileManager, TestGeneratePartitions) {
	
	this->generatePartitions();
	AdjacencyMap adj_mat;
	for (auto& i : tiles_) {
		if (i.spots[0].is_empty()) continue;
		for (int j = 1; j < i.spots.size(); ++j) {
			if (i.spots[j].is_empty()) continue;
			
			if (i.type == GridTile::disconnector) {
				if (i.spots[j].owner == i.spots[0].owner && i.spots[j].subowner != i.spots[0].subowner) {
					adj_mat[{ i.spots[j].owner, i.spots[j].subowner }];
					adj_mat[{ i.spots[0].owner, i.spots[0].subowner }];
				}
				continue;
			}
			if (i.spots[j].owner != i.spots[0].owner || i.spots[j].subowner != i.spots[0].subowner) {
				adj_mat[{i.spots[0].owner, i.spots[0].subowner}].push_back({ i.spots[j].owner,i.spots[j].subowner });
				adj_mat[{ i.spots[j].owner, i.spots[j].subowner }].push_back({ i.spots[0].owner, i.spots[0].subowner });
			}
			
		}


	}
	auto correct = FindConnectedComponents(adj_mat);

	for (auto& i : correct) {
		std::sort(i.begin(), i.end());
	}
	for (auto& i : this->partitions) {
		std::sort(i.begin(), i.end());
	}
	std::sort(correct.begin(),correct.end());
	std::sort(partitions.begin(), partitions.end());

	EXPECT_EQ(correct.size(), partitions.size());
	for (int i = 0; i < correct.size(); ++i) {
		EXPECT_TRUE(correct[i] == partitions[i]);
	}
	

	std::vector<std::vector<int>> visited(NUM_OF_MOLECULES);
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		visited[i] = std::vector<int>(number_of_submolecules[i],0);

	}
	for (auto& i : partitions) {
		for (auto& j : i) {
			++visited[j.first][j.second];
		}
	}
	

	
	//every molecule should appear exactly once in the partitions unless the contents of that molecule is not being altered, in which case it should be absent
	for (int i = 0; i < visited.size(); ++i) {
		if (!connected[i] && visited[i].size() == 1) {
			EXPECT_EQ(visited[i][0], 0);
		}
		else {
			for (auto& j : visited[i]) {
				EXPECT_EQ(j, 1);
			}
		}
	}




}

TEST_F(MockTileManager, TestGenerateConnectivityMap) {
	std::vector<std::unordered_set<int>> adj_mat(NUM_OF_MOLECULES);
	for (auto& i : tiles_) {
		if (i.type == GridTile::disconnector || i.spots[0].is_empty()) continue;
		for (int j = 0; j < i.spots.size(); ++j) {
			if (i.spots[j].is_empty()) continue;

			adj_mat[i.spots[j].owner].insert(i.spots[0].owner);
			adj_mat[i.spots[0].owner].insert(i.spots[j].owner);
		}
	}

	std::vector<int> correct(NUM_OF_MOLECULES, -1);

	std::vector<bool> visited(NUM_OF_MOLECULES, false);
	
	std::queue<int> queue;
	
	int N = 0;
	
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (visited[i]) {
			continue;
		}
		queue.push(i);
		while (!queue.empty()) {
			int current = queue.front();
			queue.pop();
			if (visited[current]) {
				continue;
			}
			visited[current] = true;
			correct[current] = N;
			for (auto& connection : adj_mat[current]) {
				queue.push(connection);
			}
		}

		++N;

	}


	std::vector<int> Counts_(N,0);
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (correct[i] != -1) {
			++Counts_[correct[i]];
		}
	}
	
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (correct[i] == -1) continue;
		//if an entire partition contains only a single molecule, it is trivial
		if (Counts_[correct[i]] == 1) {
			correct[i] = -1;
		}


	}


	std::vector<int> correction(NUM_OF_MOLECULES);
	generateConnectivityMap();
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (connectivityMap[i] == -1) continue;
		correction[connectivityMap[i]] = correct[i];
	}
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (connectivityMap[i] == -1 || correct[i] == -1) {
			if (connectivityMap[i] != correct[i]) {
				std::cerr << i << std::endl;
			}
			EXPECT_EQ(connectivityMap[i], correct[i]);
		}
		else {
			EXPECT_EQ(correction[connectivityMap[i]], correct[i]);
		}
	}

	std::unordered_map<int, int> trivialCounts;
	for (auto& i : TrivialTasks) {
		++trivialCounts[i];
	}
	for (auto& i : trivialCounts) {
		EXPECT_EQ(i.second, 1);
	}
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (number_of_submolecules[i] != 1 && !connected[i]) {
			EXPECT_TRUE(trivialCounts.find(i) != trivialCounts.end());
		}
	}

} 

TEST_F(MockTileManager, TestGenerateTasks) {
	generateConnectivityMap();
	generatePartitions();
	generateTasks();
	std::vector<int> count(NUM_OF_MOLECULES,0);
	for (auto& i : tasks) {
		
		for (auto& j : i) {
			for (auto& k : partitions[j]) {
				EXPECT_EQ(count[k.first], 0);
			}
		}

		for (auto& j : i) {
			for (auto& k : partitions[j]) {
				++count[k.first];
			}
		}
		for (auto& j : i) {
			for (auto& k : partitions[j]) {
				EXPECT_EQ(count[k.first], number_of_submolecules[k.first]);
			}
		}
	}
	for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
		if (!connected[i]) {
			EXPECT_EQ(count[i], 0);
		}
		else {
			EXPECT_EQ(count[i], number_of_submolecules[i]);
		}
	}

}

TEST_F(MockTileManager, TestConfigureTasks) {
	generateConnectivityMap();
	generatePartitions();
	generateTasks();

	int best_possible = 0;
	int what_we_got = 0;

	for (int task = 0; task < tasks.size(); ++task) {
		
		auto& partition_list = tasks[task];
		std::unordered_set<int> mols;

		int num_of_partitions = partition_list.size();
		std::vector<int> total_partition_costs(num_of_partitions,0);

		for (auto& index : partition_list) {
			for (auto& mol : partitions[index]) {
				mols.insert(mol.first);
			}
		}

		for (int i = 0; i < num_of_partitions; ++i) {
			for (auto& mol : partitions[partition_list[i]]) {
				total_partition_costs[i] += partitionSizes[mol.first][mol.second];
			}
		}

		int num_of_molecules = mols.size();

		std::unordered_map<int, int> indexMap;
		int temp_var = 0;
		for (auto& i : mols) {
			indexMap[i] = temp_var;
			++temp_var;
		}
		int MAXSIZE = std::max(num_of_molecules, num_of_partitions);
		std::vector<std::vector<int>> edges(MAXSIZE + 1,std::vector<int>(MAXSIZE + 1,0));
		/*for (int i = 0; i < num_of_molecules - num_of_partitions; ++i) {
			for (int j = 0; j < num_of_molecules; ++j) {
				edges[j + 1][1 + i + num_of_partitions] = 0;
			}
		}*/
		for (int i = 0; i < num_of_partitions - num_of_molecules; ++i) {
			for (int j = 0; j < num_of_partitions; ++j) {
				edges[1 + i + num_of_molecules][1 + j] = total_partition_costs[j];

			}
		}

		for (int i = 0; i < num_of_partitions; ++i) {
			std::vector<int> costs(num_of_molecules, total_partition_costs[i]);
			for (auto& j : partitions[partition_list[i]]) {
				costs[indexMap[j.first]] -= partitionSizes[j.first][j.second];
			}
			for (int j = 0; j < num_of_molecules; ++j) {
				edges[j + 1][i + 1] = costs[j];
			}
		}
		std::vector<int> inverse(num_of_molecules);
		for (auto& i : indexMap) {
			inverse[i.second] = i.first;
		}
		auto output = hungarian(edges);
		
		for (auto &i : output) {
			if (i.first  > num_of_molecules) {
				continue;
			}
			if (i.second > num_of_partitions) {
				continue;
			}
			int part = partition_list[i.second - 1];
			int mol = inverse[i.first - 1];
			for (auto& j : partitions[part]) {
				if (j.first == mol) {
					best_possible += partitionSizes[j.first][j.second];
				}

			}


		}

	}
	for (int i = 0; i < tasks.size(); ++i) {
		configureTask(i);
	}

	for(int i = 0; i < partitions.size(); ++i) {
		if (partitionLinks[i] == -1) continue;
		int molecule = partitionLinks[i];
		for (auto& j : partitions[i]) {
			if (j.first == molecule) {
				what_we_got += partitionSizes[j.first][j.second];
			}
		}
	}

	EXPECT_TRUE(what_we_got != 0);
	EXPECT_LE(what_we_got, best_possible);
	double ratio = double(best_possible) / double(what_we_got);
	EXPECT_GE(ratio, 0.9);


	for (int i = 0; i < partitions.size(); ++i) {
		//if a partition does not cannibalize any molecule, it must be because every molecule it intersects has already been taken
		//otherwise it would be more optimal for the partition to cannibalize that molecule
		if (partitionLinks[i] == -1) {
			for (auto& j : partitions[i]) {
				if (!connected[j.first]  && connectivityMap[j.first] == -1) {
					EXPECT_TRUE(moleculeOwners[j.first] == -1);
				}
				else {
					EXPECT_TRUE(moleculeOwners[j.first] != -1);
					EXPECT_TRUE(moleculeOwners[j.first] != i);
				}
			}
		}
		else {
			EXPECT_EQ(moleculeOwners[partitionLinks[i]], i);
		}



	}
	std::vector<int> owners(partitions.size(), 0);


	//every molecule is either not cannibalized, or cannibalized by exactly one partition
	for (auto& i : moleculeOwners) {
		if (i != -1) {
			++owners[i];
		}
	}
	for (auto& i : owners) {
		if (i != 0) {
			EXPECT_EQ(i, 1);
		}
	}
}




class Mechanism_Molecule_Test_Fixtures : public MockFixture, public testing::Test {
public:
	std::unordered_set<Geometry::grid_coordinate> TileOccupied;
	std::unordered_set<Geometry::grid_coordinate> MolOccupied;
	std::vector<Molecule> Molecules;
	std::vector<Graph::Geometry_Graph> graphs;
	std::vector<Geometry::Coordinate_Transform> Transforms;
	std::vector<std::vector<int>> defectMaps;

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
			tiles_.push_back({casted});
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
		std::uniform_int_distribution translation_dist(MIN_RANGE,MAX_RANGE);

		for (int i = 0; i < NUM_OF_MOLECULES; ++i) {
			int size = mol_dist(gen);
			Geometry::Coordinate_Transform T;
			int rot = rotation_dist(gen);
			for (int j = 0; j < rot; ++j) {
				T.Rotate();
			}
			T.translate({translation_dist(gen),translation_dist(gen)});
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
			std::vector<int> replacementMap(basenodes.size(),0);
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
			defectMaps.push_back(replacementMap);
		}

		moleculeOwners.resize(Molecules.size(), -1);
		TESTING_MOLECULEMANGAER_OVERRIDE::setNumber(Molecules.size());
		partitionSizes.resize(Molecules.size());
		kdTree.build();
		StealInstance();
	}


};

TEST_F(Mechanism_Molecule_Test_Fixtures, Testquerying ) {
	std::unordered_map<Geometry::grid_coordinate, int> counts;
	for (auto& i : tiles_) {
		for (auto& j : i.spots) {
			++counts[j.location];
		}
	}

	std::unordered_map<Geometry::grid_coordinate,int> molCounts;

	for (int i = 0; i < Transforms.size(); ++i) {
		auto T = Transforms[i];
		EXPECT_EQ(graphs[i].nodes.size(), graphs[i].coord_map[0].size());


		for (auto& j : graphs[i].nodes) {
			++molCounts[T(j.coord)];
		}
		for (auto& j : graphs[i].coord_map[0]) {
			EXPECT_EQ(j.first, graphs[i].nodes[j.second].coord); 
		}


	}

	//make sure our test fixture is valid, ie the tiles are non intersecting 
	for (auto& i : molCounts) {
		EXPECT_EQ(i.second, 1);
	}
	//make sure our molecule data is valid, ie the molecules are non intersecting
	for (auto& i : counts) {
		EXPECT_EQ(i.second, 1);
	}
	for (int i = 0; i < Molecules.size(); ++i) {
		query(Molecules[i], i);
	}
	for (auto& i : tiles_) {
		Geometry::grid_coordinate origin = i.spots[0].location;
		for (auto& j : i.spots) {
			int count = 0;
			if (j.owner == -1) {
				for (int k = 0; k < graphs.size(); ++k) {
					auto T = Transforms[k].inverse();
					if (graphs[k].coord_map[0].find(T(j.location)) != graphs[k].coord_map[0].end()) {

						//if a molecule has chosen not to call back a tile spot, it must because the tile is either a disconnector, or 
						//the molecule occupies both this spot and the 0-spot.

						EXPECT_TRUE(j.location != origin || i.type == GridTile::disconnector);
						EXPECT_TRUE(k == i.spots[0].owner || i.type == GridTile::disconnector);
						//EXPECT_TRUE(graphs[k].coord_map[0].find(T(j.location)) == graphs[k].coord_map[0].end());
					}
					//EXPECT_TRUE(graphs[k].coord_map[0].find(T(j.location)) == graphs[k].coord_map[0].end());
				}
			}
			else {
				for (int k = 0; k < graphs.size(); ++k) {
					auto T = Transforms[k].inverse();
					if (k == j.owner) {
						EXPECT_TRUE(graphs[k].coord_map[0].find(T(j.location)) != graphs[k].coord_map[0].end());
						continue;
					}
					
					EXPECT_TRUE(graphs[k].coord_map[0].find(T(j.location)) == graphs[k].coord_map[0].end());
				}
			}
			
		}
	}
}


TEST_F(Mechanism_Molecule_Test_Fixtures, UpdateConnectionsTest) {
	std::unordered_map<Geometry::grid_coordinate, int> Coords;
	Coords.reserve(NUM_OF_TILES * 7);
	for (int i = 0; i < Molecules.size(); ++i) {
		auto T = Transforms[i];
		for (auto& j : graphs[i].nodes) {
			Coords[T(j.coord)] = i;
		}
	}
	for (int i = 0; i < Molecules.size(); ++i) {
		query(Molecules[i], i);
	}
	std::vector<const Graph::Collision_Leaf*> nodes_;
	std::vector<Geometry::grid_coordinate> deltas = { {0,1},{1,0},{1,-1},{0,-1},{-1,0},{-1,1} };
	for (int i = 0; i < Molecules.size(); ++i) {
		nodes_.push_back(TESTING_MOLECULEMANGAER_OVERRIDE::getHierarchy(Molecules[i]).getNodes()[0].data());
	}



	for (auto& i : tiles_) {
		if (Coords.find(i.spots[0].location) != Coords.end()) {
			int owner = Coords.at(i.spots[0].location);
			for (int j = 1; j < i.spots.size(); ++j) {
				if (Coords.find(i.spots[j].location) != Coords.end() && Coords.at(i.spots[j].location) == owner ) {
					auto T = Transforms[owner].inverse();
					auto loc1 = T(i.spots[0].location);
					auto loc2 = T(i.spots[j].location);
					int pos1 = defectMaps[owner][graphs[owner].coord_map[0][loc1]];
					int pos2 = defectMaps[owner][graphs[owner].coord_map[0][loc2]];
					auto delta = loc2 - loc1;
					int child;
					if (delta == deltas[0]) { child = 1; }
					else if (delta == deltas[1]){ child = 2;}
					else if (delta == deltas[2]){ child = 3;}
					else if (delta == deltas[3]){ child = 4;}
					else if (delta == deltas[4]){ child = 5;}
					else { child = 6; }

					if (i.type == GridTile::connecter) {
						for (int k = 1; k < 7; ++k) {
							if (k == child) continue;
							EXPECT_TRUE(nodes_[owner][pos1].children[k] != pos2);
							EXPECT_TRUE(nodes_[owner][pos2].children[1 + (2 + k) % 6] != pos1);
						}

						EXPECT_EQ(nodes_[owner][pos1].children[child], pos2);
						EXPECT_EQ(nodes_[owner][pos2].children[1 + (2 + child) % 6], pos1);
					}
					else {
						for (int k = 1; k < 7; ++k) {
							EXPECT_TRUE(nodes_[owner][pos1].children[k] != pos2);
							EXPECT_TRUE(nodes_[owner][pos2].children[1 + (2 + k) % 6] != pos1);
						}

						EXPECT_EQ(nodes_[owner][pos1].children[child], -1);
						EXPECT_EQ(nodes_[owner][pos2].children[1 + (2 + child) % 6], -1);
					}




				}
			}
		}
	}
}




TEST_F(Mechanism_Molecule_Test_Fixtures, TestCallbackConnections) {
	std::vector<const Graph::Collision_Leaf*> nodes_;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dist(0, 1);
	constexpr double chance_of_defect = 0.1;

	

	



	for (int i = 0; i < Molecules.size(); ++i) {
		query(Molecules[i], i);
	}
	for (int i = 0; i < Molecules.size(); ++i) {
		nodes_.push_back(TESTING_MOLECULEMANGAER_OVERRIDE::getHierarchy(Molecules[i]).getNodes()[0].data());
	}
	std::unordered_map<Geometry::grid_coordinate, GridTile::spot*> callbacks;
	std::unordered_map<Geometry::grid_coordinate, std::pair<int,int>> TESTcallbacks;
	std::vector<std::vector<int>> sizes(Molecules.size());

	for (auto& i : tiles_) {
		if (i.type == GridTile::disconnector) continue;
		for (auto& j : i.spots) {
			if (j.is_empty()) continue;
			callbacks[j.location] = &j;
		}
	}
	std::vector<std::vector<int>> inverses(Molecules.size());

	std::vector<std::vector<int>> subgroupMap(Molecules.size());

	for (int i = 0; i < Molecules.size(); ++i) {
		std::vector<int> &inverses_ = inverses[i];
		inverses_.resize((defectMaps[i].back() + 1));
		for (int k = 0; k < defectMaps[i].size(); ++k) {
			inverses_[defectMaps[i][k]] = k;
		}


		int subgroupID = 0;
		std::vector<bool> visited(graphs[i].nodes.size(), false);
		auto T = Transforms[i];
		for (int j = 0; j < graphs[i].nodes.size(); ++j) {
			int currentSubgroupSize = 0;
			std::queue<int> queue;
			if (visited[j]) continue;
			queue.push(j);
			while (!queue.empty()) {
				int current = queue.front();
				queue.pop();
				if (visited[current]) {
					continue;
				}
				++currentSubgroupSize;
				visited[current] = true;
				TESTcallbacks[T(graphs[i].nodes[current].coord)] = { i,subgroupID };
				for (int k = 1; k < 7; ++k) {
					if (nodes_[i][defectMaps[i][current]].children[k] != -1) {
						queue.push(inverses_[nodes_[i][defectMaps[i][current]].children[k]]);
					}
				}
			}
			++subgroupID;
			sizes[i].push_back(currentSubgroupSize);
			subgroupMap[i].push_back(-1);
		}
	}

	for (int i = 0; i < Molecules.size(); ++i) {
		Molecules[i].callbackConnections(i);
	}

	auto& PartitionSizes = ACCESSOR.getpartitionSizes(instance());
	for (int i = 0; i < Molecules.size(); ++i) {
		auto& components = TESTING_MOLECULEMANGAER_OVERRIDE::getComponentMap(Molecules[i]);
		if (components.size() != 0) {
			EXPECT_EQ(components.size(), sizes[i].size());
			for (int j = 0; j < components.size(); ++j) {
				auto subgroupCoord = Transforms[i](nodes_[i][components[j]].coord);
				EXPECT_TRUE(TESTcallbacks.find(subgroupCoord) != TESTcallbacks.end());
				if (TESTcallbacks.find(subgroupCoord) != TESTcallbacks.end()) {
					subgroupMap[i][TESTcallbacks.at(subgroupCoord).second] = j;
				}
			}
			for (int j = 0; j < components.size(); ++j) {
				EXPECT_TRUE(subgroupMap[i][j] != -1);
				EXPECT_EQ(PartitionSizes[i][subgroupMap[i][j]], sizes[i][j]);
			}
		}
		//if components has 0 size then the molecules must not have any new connections with another molecule, and have no disconnection with itself
		else {
			EXPECT_EQ(sizes[i].size(), 1);
			for (auto& j : tiles_) {
				int owner;
				if (TESTcallbacks.find(j.spots[0].location) == TESTcallbacks.end()) {
					continue;
				}
				owner = TESTcallbacks[j.spots[0].location].first;
				if (owner != i && j.type == GridTile::disconnector) {
					continue;
				}
				for (int k = 1; k < j.spots.size(); ++k) {
					if (TESTcallbacks.find(j.spots[k].location) == TESTcallbacks.end()) {
						continue;
					}
					int neighbor = TESTcallbacks[j.spots[k].location].first;
					if (j.type == GridTile::disconnector) {
						EXPECT_TRUE(neighbor != i);
					}
					else {
						EXPECT_TRUE(neighbor != owner || neighbor != i);
					}
				}
			}
		}
	}
	for (auto& i : tiles_) {
		for (auto& k : i.spots) {
			if (k.is_empty()) continue;
			EXPECT_TRUE(TESTcallbacks.find(k.location) != TESTcallbacks.end());
			if (TESTcallbacks.find(k.location) != TESTcallbacks.end()) {
				auto correct = TESTcallbacks[k.location];
				EXPECT_EQ(correct.first, k.owner);
				EXPECT_EQ(k.subowner, subgroupMap[correct.first][correct.second]);
			}
		}
	}
}
