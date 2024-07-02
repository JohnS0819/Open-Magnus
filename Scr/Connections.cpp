#include "Connections.hpp"



void deepCopyDfs(int position, std::vector<Graph::Collision_Leaf>& graph, int& index, ConnectionManager::compressedNode* output, std::unordered_map<int, int>& map, std::vector<int>& available) {
	//make sure node has not yet been visited
	if (graph[position].children[0] == -1) return;

	available.push_back(position);

	graph[position].children[0] = -1;
	int current = index;
	for (int i = 1; i < 7; ++i) {

		if (graph[position].children[i] == -1) {
			output[current].children[i] = -1;
			continue;
		}
		if (map.find(graph[position].children[i]) == map.end()) {
			++index;
			map[graph[position].children[i]] = index;
			output[current].children[i] = index;
			deepCopyDfs(graph[position].children[i], graph, index, output, map, available);
		}
		else {
			output[current].children[i] = map[graph[position].children[i]];
		}
	}
	output[current].coord = graph[position].coord;

	//output[current].collision = graph[position].collision;
}


void deepCopyDfs(int position, std::vector<Graph::Collision_Leaf>& graph, int& index, std::vector<Graph::Collision_Leaf> &output, std::unordered_map<int, int>& map) {
	//make sure node has not yet been visited
	if (graph[position].children[0] == -1) return;

	graph[position].children[0] = -1;
	int current = index;
	output[current].coord = graph[position].coord;
	output[current].collision = graph[position].collision;
	for (int i = 1; i < 7; ++i) {

		if (graph[position].children[i] == -1) {
			output[current].children[i] = -1;
			continue;
		}
		if (map.find(graph[position].children[i]) == map.end()) {
			++index;
			map[graph[position].children[i]] = index;
			output[current].children[i] = index;
			deepCopyDfs(graph[position].children[i], graph, index, output, map);
		}
		else {
			output[current].children[i] = map[graph[position].children[i]];
		}
	}
	

}


ConnectionManager::ConnectionManager() {}



std::vector<std::vector<Graph::Collision_Leaf>>& ConnectionManager::AccessNodes(Molecule& M) {
	return M.Colliders.nodes;

}
std::vector<std::unordered_map<Geometry::grid_coordinate, int>>& ConnectionManager::AccessCoordMap(Molecule& M) {
	return M.Colliders.coord_map;
}


ConnectionManager& ConnectionManager::instance() {
	static ConnectionManager singleton;
	return singleton;
}

void dfsRemoval(int position, std::vector<Graph::Collision_Leaf>& graph, std::vector<int>& available,
	std::unordered_map<Geometry::grid_coordinate, int>& map, std::unordered_map<Geometry::grid_coordinate, int>& parentMap,
	std::vector<Graph::Collision_Leaf> &parent, std::unordered_set<int> &update) {

	if (map.find(graph[position].coord) == map.end()) return;
	map.erase(graph[position].coord);
	//graph[position].children[0] = -1;
	available.push_back(position);
	auto out = Graph::Collision_Hierarchy::findParent(graph[position].coord);
	int pos = parentMap[out.first];
	update.insert(pos);
	parent[pos].children[out.second] = -1;
	for (int j = 1; j < 7; ++j) {
		if (graph[position].children[j] == -1) continue;
		dfsRemoval(graph[position].children[j], graph, available, map, parentMap, parent, update);
	}

}

void ConnectionManager::completeTrivial(Molecule& molecule, int moleculeID) {
	int N = TileManager::instance().partitionSizes[moleculeID].size();
	int parent = TileManager::instance().partitionSizes[moleculeID][N - 1];
	for (int i = 0; i < N - 1; ++i) {
		if (i == parent) {
			trim(molecule, moleculeID, i);
		}
		else {
			MoleculeManager::instance().insert(new Molecule(extractDirectly(molecule, moleculeID, i)));
		}
	}

}


void ConnectionManager::trim(Molecule& molecule, int moleculeID, int component) {
	auto& available = molecule.available_;
	auto& nodes = AccessNodes(molecule);
	auto& map = AccessCoordMap(molecule);

	std::unordered_set<int> update;
	std::unordered_set<int> buffer;

	available[0].reserve(nodes[0].size() - TileManager::instance().partitionSizes[moleculeID][component]);
	
	for (int i = 0; i < molecule.componentMap.size(); ++i) {
		if (i == component) continue;
		int begin = molecule.componentMap[i];
		

		dfsRemoval(begin, nodes[0], available[0], map[0], map[1], nodes[1], update);
	}

	

	molecule.bubbleUpdate(1, update, buffer);

}

Molecule ConnectionManager::extractDirectly(Molecule& molecule, int MoleculeID, int component) {
	std::unordered_map<int, int> index_map;
	Molecule output;
	output.coordTransform = molecule.coordTransform;
	output.inverse_map = molecule.inverse_map;
	output.Orientation = molecule.Orientation;
	auto& nodes = output.Colliders.nodes[0];
	auto& coordmap = output.Colliders.coord_map[0];
	coordmap.reserve(TileManager::instance().partitionSizes[MoleculeID][component]);
	nodes.resize(TileManager::instance().partitionSizes[MoleculeID][component]);
	int start = molecule.componentMap[component];
	index_map[start] = 0;
	int index = 0;
	deepCopyDfs(start, molecule.Colliders.nodes[0], index, nodes, index_map);
	for (int i = 0; i < nodes.size(); ++i) {
		output.Colliders.coord_map[0][nodes[i].coord] = i;
	}
	if (output.Colliders.nodes.size() == 0) {
		std::cerr << "no" << std::endl;
	}
	while (output.Colliders.nodes.back().size() > 1) {
		output.Colliders.subdivide();
	}
	output.available_.resize(output.Colliders.nodes.size());
	return output;

}


void ConnectionManager::extractAndCompress(Molecule& molecule, int MoleculeID) {
	std::unordered_map<int, int> index_map;
	int partitionGroup = TileManager::instance().connectivityMap[MoleculeID];
	int N = molecule.componentMap.size();
	auto* bufferRegion = buffer_[partitionGroup].data();

	molecule.start_of_update_ = molecule.available_[0].size();

	moleculeTransforms[MoleculeID] = molecule.coordTransform;

	for (int i = 0; i < N; ++i) {
		if (offsets_[MoleculeID][i] == -1) continue;
		int index = offsets_[MoleculeID][i];
		index_map[molecule.componentMap[i]] = index;
		deepCopyDfs(molecule.componentMap[i], molecule.Colliders.nodes[0], index, bufferRegion, index_map, molecule.available_[0]);

	}
}

void ConnectionManager::update(int partitionID, Molecule& molecule) {
	std::unordered_set<int> updates;
	if (molecule.Colliders.nodes.size() == 1) {
		molecule.Colliders.nodes.push_back({});
		molecule.Colliders.coord_map.push_back({});
		molecule.available_.push_back({});
		if (!molecule.Colliders.coord_map[0].empty()) {
			auto& child = molecule.Colliders.coord_map[0].begin()->second;
			auto parent = Graph::Collision_Hierarchy::findParent(molecule.Colliders.nodes[0][child].coord);
			molecule.Colliders.coord_map[1][parent.first] = 0;
			molecule.Colliders.nodes[1].push_back(parent.first);
			molecule.Colliders.nodes[1][0].children[parent.second] = child;
			
		}
				
	}
	if (molecule.Colliders.coord_map[1].size() == 1) {
		updates.insert(molecule.Colliders.coord_map[1].begin()->second);
	}
	auto& partition = TileManager::instance().partitions[partitionID];
	int moleculeID = TileManager::instance().partitionLinks[partitionID];

	int groupID = TileManager::instance().connectivityMap[partition[0].first];

	auto &base = molecule.Colliders;

	auto& available = molecule.available_[0];

	std::unordered_set<int> updatebuffer;
	auto& buffer = buffer_[groupID];

	int N = available.size();
	//process removals at the first level and add their parent nodes to the update list
	for (int i = molecule.start_of_update_; i < N; ++i) {
		auto& current = base.nodes[0][available[i]];
		base.coord_map[0].erase(current.coord);
		auto parent = Graph::Collision_Hierarchy::findParent(current.coord);
		int pos = base.coord_map[1][parent.first];
		updates.insert(pos);
		base.nodes[1][pos].children[parent.second] = -1;
	}
	int maxIndex = partition.size();
	for (int currentIndex = 0; currentIndex < maxIndex; ++currentIndex) {
		int owner = partition[currentIndex].first;
		int subowner = partition[currentIndex].second;
		if (owner == moleculeID) {
			continue;
		}

		int sizeofSubgraph = TileManager::instance().partitionSizes[owner][subowner];
		int startofSubgraph = this->offsets_[owner][subowner];
		int endofSubgraph = startofSubgraph + sizeofSubgraph;
		Geometry::Coordinate_Transform transform = molecule.inverse_map * moleculeTransforms[owner];
		if (N == 0) {
			int bufferstart = base.nodes[0].size() - startofSubgraph;
			for (int i = startofSubgraph; i < endofSubgraph; ++i) {
				base.nodes[0].push_back({});
				auto& current = base.nodes[0].back();
				current.coord = transform(buffer[i].coord);

				base.coord_map[0][current.coord] = base.nodes[0].size() - 1;

				current.collision.center_ = 
					{ (float)current.coord.i + 0.5f * (float)current.coord.j
					, (float)current.coord.j * 0.866025403784f };

				current.collision.radius_ = ATOMIC_RADIUS;
				//we've rotated our reference frame so we have to adjust the slot that a connection fills as well
				int trace = transform.x_mat.i + transform.y_mat.j;
				int rotationAdjustment;
				if (trace > 0) {
					rotationAdjustment = transform.x_mat.i + 5 * transform.y_mat.j;
				}
				else {
					rotationAdjustment = 3 + transform.y_mat.j - transform.x_mat.i;
				}


				for (int j = 1; j < 7; ++j) {
					int correctSlot = 1 + (j + 5 + rotationAdjustment) % 6;
					if (buffer[i].children[j] == -1) {
						current.children[correctSlot] = -1;
						continue;
					}
					
					current.children[correctSlot] = buffer[i].children[j] + bufferstart;
				}
				auto parent = Graph::Collision_Hierarchy::findParent(current.coord);
				if (base.coord_map[1].find(parent.first) == base.coord_map[1].end()) {
					if (molecule.available_[1].empty()) {
						base.coord_map[1][parent.first] = base.nodes[1].size();
						base.nodes[1].push_back({});
						base.nodes[1].back().coord = parent.first;
						base.nodes[1].back().children[parent.second] = bufferstart + i;
						updates.insert(base.nodes[1].size() - 1);
					}
					else {
						int loc = molecule.available_[1].back();
						molecule.available_[1].pop_back();
						base.nodes[1][loc].coord = parent.first;
						base.nodes[1][loc].children[parent.second] = bufferstart + i;
						base.coord_map[1][parent.first] = loc;
						updates.insert(loc);
					}


				}
				else {
					int loc = base.coord_map[1][parent.first];
					base.nodes[1][loc].children[parent.second] = bufferstart + i;
					updates.insert(loc);
				}
			}
		}
		else if (N >= sizeofSubgraph) {
			int adjustedStartofAvailable = N + startofSubgraph - 1;
			for (int i = startofSubgraph; i < endofSubgraph; ++i) {

				auto& current = base.nodes[0][available[adjustedStartofAvailable - i]];
				current.coord = transform(buffer[i].coord);

				base.coord_map[0][current.coord] = available[adjustedStartofAvailable - i];

				current.collision.center_ =
				{ (float)current.coord.i + 0.5f * (float)current.coord.j
				, (float)current.coord.j * 0.866025403784f };

				current.collision.radius_ = ATOMIC_RADIUS;
				//we've rotated our reference frame so we have to adjust the slot that a connection fills as well
				int trace = transform.x_mat.i + transform.y_mat.j;
				int rotationAdjustment;
				if (trace > 0) {
					rotationAdjustment = transform.x_mat.i + 5 * transform.y_mat.j;
				}
				else {
					rotationAdjustment = 3 + transform.y_mat.j - transform.x_mat.i;
				}


				for (int j = 1; j < 7; ++j) {
					int correctSlot = 1 + (j + 5 + rotationAdjustment) % 6;
					if (buffer[i].children[j] == -1) {
						current.children[correctSlot] = -1;
						continue;
					}
					
					current.children[correctSlot] = available[adjustedStartofAvailable -  buffer[i].children[j]];
				}
				auto parent = Graph::Collision_Hierarchy::findParent(current.coord);
				if (base.coord_map[1].find(parent.first) == base.coord_map[1].end()) {
					if (molecule.available_[1].empty()) {
						base.coord_map[1][parent.first] = base.nodes[1].size();
						base.nodes[1].push_back({});
						base.nodes[1].back().coord = parent.first;
						base.nodes[1].back().children[parent.second] = available[adjustedStartofAvailable - i];
						updates.insert(base.nodes[1].size() - 1);
					}
					else {
						int loc = molecule.available_[1].back();
						molecule.available_[1].pop_back();
						base.nodes[1][loc].coord = parent.first;
						base.nodes[1][loc].children[parent.second] = available[adjustedStartofAvailable - i];
						base.coord_map[1][parent.first] = loc;
						updates.insert(loc);
					}


				}
				else {
					int loc = base.coord_map[1][parent.first];
					base.nodes[1][loc].children[parent.second] = available[adjustedStartofAvailable - i];
					updates.insert(loc);
				}
			}
			N -= sizeofSubgraph;
			available.resize(N);

		}
		else{
			int adjustedStartofAvailable = N + startofSubgraph - 1;
			int initialSize = base.nodes[0].size();
			for (int i = startofSubgraph; i < endofSubgraph; ++i) {
				int current_position;
				if (i > adjustedStartofAvailable) {
					base.nodes[0].push_back({});
					current_position = base.nodes[0].size() - 1;
				}
				else {
					current_position = available[adjustedStartofAvailable - i];
				}
				auto& current = base.nodes[0][current_position];
				current.coord = transform(buffer[i].coord);

				base.coord_map[0][current.coord] = current_position;

				current.collision.center_ =
				{ (float)current.coord.i + 0.5f * (float)current.coord.j
				, (float)current.coord.j * 0.866025403784f };

				current.collision.radius_ = ATOMIC_RADIUS;
				//we've rotated our reference frame so we have to adjust the slot that a connection fills as well
				int trace = transform.x_mat.i + transform.y_mat.j;
				int rotationAdjustment;
				if (trace > 0) {
					rotationAdjustment = transform.x_mat.i + 5 * transform.y_mat.j;
				}
				else {
					rotationAdjustment = 3 + transform.y_mat.j - transform.x_mat.i;
				}


				for (int j = 1; j < 7; ++j) {
					int correctSlot = 1 + (j + 5 + rotationAdjustment) % 6;

					if (buffer[i].children[j] == -1) {
						current.children[correctSlot] = -1;
						continue;
					}
					
					if (buffer[i].children[j] > adjustedStartofAvailable) {
						current.children[correctSlot] = initialSize + buffer[i].children[j] - adjustedStartofAvailable - 1;
					}
					else {
						current.children[correctSlot] = available[adjustedStartofAvailable - buffer[i].children[j]];
					}
					
				}
				auto parent = Graph::Collision_Hierarchy::findParent(current.coord);
				if (base.coord_map[1].find(parent.first) == base.coord_map[1].end()) {
					if (molecule.available_[1].empty()) {
						base.coord_map[1][parent.first] = base.nodes[1].size();
						base.nodes[1].push_back({});
						base.nodes[1].back().coord = parent.first;
						base.nodes[1].back().children[parent.second] = current_position;
						updates.insert(base.nodes[1].size() - 1);
					}
					else {
						int loc = molecule.available_[1].back();
						molecule.available_[1].pop_back();
						base.nodes[1][loc].coord = parent.first;
						base.nodes[1][loc].children[parent.second] = current_position;
						base.coord_map[1][parent.first] = loc;
						updates.insert(loc);
					}


				}
				else {
					int loc = base.coord_map[1][parent.first];
					base.nodes[1][loc].children[parent.second] = current_position;
					updates.insert(loc);
				}
			}
			available.resize(0);
			N = 0;

		}
	}
	molecule.bubbleUpdate(1, updates, updatebuffer);

}


void ConnectionManager::setBufferSize(int partitionGroupID) {

	buffer_[partitionGroupID].resize(partitionGroupLastPositions[partitionGroupID]);
}

void ConnectionManager::completeNonTrivial() {
	for (auto& i : TileManager::instance().tasks) {

	}
}

void ConnectionManager::setupBuffers() {
	int N = MoleculeManager::instance().number();
	offsets_.clear();
	offsets_.resize(N);
	for (int i = 0; i < N; ++i) {
		offsets_[i].resize(TileManager::instance().partitionSizes[i].size(), -1);
	}
	buffer_.clear();
	buffer_.resize(TileManager::instance().tasks.size());
	partitionGroupLastPositions.clear();
	partitionGroupLastPositions.resize(TileManager::instance().tasks.size(), 0);
	moleculeTransforms.clear();
	moleculeTransforms.resize(N);



}

void ConnectionManager::setoffsets(int moleculeID,int partitionGroupID) {

	int index = TileManager::instance().moleculeOwners[moleculeID];
	if (index != -1) {
		for (auto& i : TileManager::instance().partitions[index]) {
			if (i.first != moleculeID) {
				continue;
			}
			TileManager::instance().partitionSizes[moleculeID][i.second] = -1;
		}
	}
	for (int i = 0; i < TileManager::instance().partitionSizes[moleculeID].size(); ++i) {
		if (TileManager::instance().partitionSizes[moleculeID][i] != -1) {
			offsets_[moleculeID][i] = partitionGroupLastPositions[partitionGroupID];
			partitionGroupLastPositions[partitionGroupID] += TileManager::instance().partitionSizes[moleculeID][i];
		}
	}
}