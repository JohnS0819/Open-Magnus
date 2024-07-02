#include "Tile.hpp"
#include <numeric>


TileManager::TileManager() : kdTree(tiles_) {}

TileManager& TileManager::instance() {
	static TileManager singleton;
	return singleton;

}

void TileManager::reset() {
	for (auto& i : tiles_) {
		i.clear();
	}
	std::fill(connectivityMap.begin(), connectivityMap.end(), -1);
	tasks.clear();
	TrivialTasks.clear();
	partitionSizes.clear();
	partitionSizes.resize(MoleculeManager::instance().number());
}

void TileManager::build() {
	kdTree.clear();
	kdTree.build();
}

void TileManager::insert(const GridTile& tile) {
	tiles_.push_back(tile);
}

void Molecule::updateConnection(GridTile& tile, int id) {
	bool internal_ = false;
	bool new_connection = tile.type == GridTile::Type::connecter;
	int location = query_internal(tile.spots[0].location);

	if (tile.spots[0].is_empty() && location != -1) {
		internal_ = true;
		if (new_connection) {
			tile.spots[0].owner = id;
			connectionCallback[location] = &tile.spots[0].subowner;
		}
	}
	for (int i = 1; i < tile.spots.size(); ++i) {
		if (tile.spots[i].is_empty()) {
			int location2 = query_internal(tile.spots[i].location);
			if (location2 == -1) {
				continue;
			}
			
			if (location == -1) {
				if (new_connection) {
					connectionCallback[location2] = &tile.spots[i].subowner;
					tile.spots[i].owner = id;
				}
				continue;
			}
			auto adjusted = inverse_map.ApplyRotation(tile.spots[i].location - tile.spots[0].location);

			constexpr int correction_map[7] = { 5,6,4,0,1,3,2 };
			constexpr auto indexer = [](const Geometry::grid_coordinate& coord) {return 3 * coord.i + coord.j + 3; };
			int index = correction_map[indexer(adjusted)];
			//std::cerr << index << std::endl;
			if (new_connection) {
				Colliders.nodes[0][location].children[index] = location2;
				Colliders.nodes[0][location2].children[1 + (2 + index) % 6] = location;
			}
			else {
				ConnectionsUpdated = true;
				Colliders.nodes[0][location].children[index] = -1;
				Colliders.nodes[0][location2].children[1 + (2 + index) % 6] = -1;
			}
		}
	}
}

void Molecule::callbackConnections(int id) {
	if (connectionCallback.empty() && !ConnectionsUpdated) {
		return;
	}
	auto& nodes = Colliders.nodes[0];
	auto& baba = Colliders.coord_map[0];
	int marker = 0;
	for (auto &j : baba) {
		int i = j.second;
		Graph::markComponent(nodes, i, marker);
		if (nodes[i].children[0] == marker) {
			componentMap.push_back(i);
			TileManager::instance().partitionSizes[id].push_back(0);
			++marker;
		}
		++TileManager::instance().partitionSizes[id][nodes[i].children[0]];
	}
	for (auto& i : connectionCallback) {
		*i.second = nodes[i.first].children[0];
	}
}


void TileManager::query(Molecule& molecule, int moleculeIndex) {
	auto collider = molecule.getBoundingCircle();
	auto possible_intersections = kdTree.radiusSearch(collider.center_, collider.radius_+ 0.7f);
	for (auto& index : possible_intersections) {
		molecule.updateConnection(tiles_[index], moleculeIndex);
	}
}



void dfs(int index, const int &value, std::vector<int> &visited, const std::vector<std::unordered_set<int>> &adj_mat ) {
	if (visited[index] != -1) return;
	visited[index] = value;
	for (int i : adj_mat[index]) {
		dfs(i, value, visited, adj_mat);
	}
}



void TileManager::generateConnectivityMap() {

	int N = MoleculeManager::instance().number();
	connectivityMap.clear();
	connectivityMap.resize(N,-1);

	moleculeOwners.clear();
	moleculeOwners.resize(N, -1); 
	
	//std::fill(connectivityMap.begin(), connectivityMap.end(), -1);

	std::vector<std::unordered_set<int>> adj_mat(MoleculeManager::instance().number());

	for (auto& i : tiles_) {
		if (i.spots[0].is_empty() || i.type == GridTile::Type::disconnector) continue;
		for (int j = 1; j < i.spots.size(); ++j) {
			if (i.spots[j].is_empty() || i.spots[j].owner == i.spots[0].owner) continue;
			adj_mat[i.spots[j].owner].insert(i.spots[0].owner);
			adj_mat[i.spots[0].owner].insert(i.spots[j].owner);
		}
	}
	int index = 0;
	for (int i = 0; i < N; ++i) {
		if (connectivityMap[i] != -1) continue;
		/*if (adj_mat[i].empty() && partitionSizes[i].size() < 2) continue;*/
		if (adj_mat[i].empty()) {
			if (partitionSizes[i].size() > 1) {
				TrivialTasks.push_back(i);
			}
			continue;
		}
		dfs(i,index,connectivityMap,adj_mat);
		++index;
	}
	tasks.resize(index);

}

int TileManager::getNumberOfTasks() const {
	return tasks.size();
}

int TileManager::getNumberOfTrivialTasks() const {
	return TrivialTasks.size();

}

void TileManager::generatePartitions() {
	std::unordered_map<std::pair<int, int>, std::vector<std::pair<int,int>>* > indexDict;
	int N = 0;
	for (const GridTile& tile : tiles_) {
		if (tile.spots[0].owner == -1) {
			continue;
		}

		
		std::vector<std::pair<int, int>>* index ;
		if (indexDict.find({ tile.spots[0].owner,tile.spots[0].subowner }) == indexDict.end()) {
			/*index = new std::vector< std::pair<int, int>>({ {tile.spots[0].owner,tile.spots[0].subowner} });
			indexDict.insert({ { tile.spots[0].owner,tile.spots[0].subowner }, index });
			++N;*/
			index = nullptr;
		}
		else {
			index = indexDict[{tile.spots[0].owner, tile.spots[0].subowner}];
		}

		if (tile.type == GridTile::disconnector) {
			for (int i = 1; i < tile.spots.size(); ++i) {
				if (tile.spots[i].owner != tile.spots[0].owner || tile.spots[i].subowner == tile.spots[0].subowner) {
					continue;
				}
				if (indexDict.find({ tile.spots[i].owner,tile.spots[i].subowner }) == indexDict.end()) {
					indexDict[{ tile.spots[i].owner, tile.spots[i].subowner }] =
						new std::vector< std::pair<int, int>>({ {tile.spots[i].owner,tile.spots[i].subowner} });
					++N;
				}
				if (index == nullptr) {
					index = new std::vector< std::pair<int, int>>({ {tile.spots[0].owner,tile.spots[0].subowner} });
					indexDict.insert({ { tile.spots[0].owner,tile.spots[0].subowner }, index });
					++N;
				}
			}
			continue;
		}

		for (int i = 1; i < tile.spots.size(); ++i) {
			if (tile.spots[i].owner == -1 || tile.spots[i].owner == tile.spots[0].owner) {
				continue;
			}
			auto position = indexDict.find({ tile.spots[i].owner , tile.spots[i].subowner });
			if (index == nullptr) {
				if (position == indexDict.end()) {
					index = new std::vector< std::pair<int, int>>({ {tile.spots[0].owner,tile.spots[0].subowner},{ tile.spots[i].owner , tile.spots[i].subowner } });
					indexDict.insert({ { tile.spots[0].owner,tile.spots[0].subowner }, index });
					indexDict.insert({ { tile.spots[i].owner , tile.spots[i].subowner },index });
					++N;
					continue;
				}
				index = position->second;
				indexDict.insert({ { tile.spots[0].owner,tile.spots[0].subowner }, index });
				index->push_back({ tile.spots[0].owner,tile.spots[0].subowner });
			}
			
			if (position == indexDict.end()) {
				indexDict.insert({ { tile.spots[i].owner , tile.spots[i].subowner },index });
				index->push_back({ tile.spots[i].owner , tile.spots[i].subowner });
				continue;
			}
			else if (position->second == index) {
				continue;
			}
			else if (position->second->size() > index->size()) {
				--N;
				auto P = position->second;
				P->insert(P->end(), index->begin(), index->end());

				for (auto j = index->begin(); j != index->end(); ++j) {
					indexDict[*j] = P;
				}
				delete(index);
				index = P;
			}
			else {
				--N;
				auto P = position->second;
				index->insert(index->end(), P->begin(), P->end());

				for (auto j = P->begin(); j != P->end(); ++j) {
					indexDict[*j] = index;
				}
				delete(P);
			}
		}
	}
	partitions.resize(N);
	partitionLinks.resize(N, -1);
	std::vector<std::vector<std::pair<int, int>>*> destructor(N);
	int n = 0;
	for (auto& i : indexDict) {
		if (i.second->size() == 0) {
			continue;
		}
		destructor[n] = i.second;
		std::swap(partitions[n], *i.second);
		++n;
	}
	for (auto& i : destructor) {
		delete(i);
	}
}

void TileManager::generateTasks() {
	for (int i = 0; i < partitions.size(); ++i) {
		int index = connectivityMap[partitions[i][0].first];
		if (index == -1) continue;
		tasks[index].push_back(i);
	}

}

void TileManager::configureTask(int id) {
	auto& task = tasks[id];

	struct linkage {
		int partition_id;
		int molecule_id;
		int benefit;

		bool operator<(const linkage& a) const {
			return benefit < a.benefit;
		}

	};

	std::vector<linkage> links;
	int size = 0;
	for (const auto& index: task) {
		size += partitions[index].size();
	}
	links.reserve(size);
	for (auto& index : task) {
		std::unordered_map<int, int> values;
		for (auto& i : partitions[index]) {
			values[i.first] += partitionSizes[i.first][i.second];
		}
		for (auto& i : values) {
			//data structure is partition index, molecule id, benefit of using that molecule for the new partition 
			links.push_back({ index ,i.first, i.second});
		}


	}
	//hungarian is too slow so just use lazy approach
	std::sort(links.begin(), links.end());
	for (int i = links.size() - 1; i >= 0; --i) {
		if (moleculeOwners[links[i].molecule_id] == -1 && partitionLinks[links[i].partition_id] == -1) {
			//MoleculeManager::instance().delayDestruction(links[i].molecule_id);
			moleculeOwners[links[i].molecule_id] = links[i].partition_id;
			partitionLinks[links[i].partition_id] = links[i].molecule_id;
		}
	}
}


void TileManager::configureTrivialTask(int id) {
	if (id >= TrivialTasks.size()) return;
	int moleculeID = TrivialTasks[id];
	auto max_element = std::max_element(partitionSizes[id].begin(), partitionSizes[id].end());
	int position = max_element - partitionSizes[id].begin();
	partitionSizes[moleculeID].push_back(position);
}

template<>
void Rendering::Render_Window::draw(const GridTile& tile) {
	for (auto& i : tile.spots) {
		draw(i);
	}
}

const std::vector<int>& TileManager::getTrivialTasks() const {
	return TrivialTasks;
}

template<>
void Rendering::Render_Window::draw(const GridTile::spot& tile) {
	Geometry::coordinate center = tile.location;
	center *= GAMESPACE_TO_RENDERSPACE;
	constexpr double Y0 = -0.5773502691;
	constexpr double Y3 = 0.5773502691;
	constexpr double Y1 = -0.28867513459;
	constexpr double Y2 = 0.28867513459;
	constexpr double X1 = 0.5;
	constexpr double X2 = -0.5;

	sf::ConvexShape shape(6);
	shape.setPoint(0, sf::Vector2f(center.x,center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y0));
	shape.setPoint(1, sf::Vector2f(center.x + 0.95 * GAMESPACE_TO_RENDERSPACE * X1, center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y1));
	shape.setPoint(2, sf::Vector2f(center.x + 0.95 * GAMESPACE_TO_RENDERSPACE * X1, center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y2));
	shape.setPoint(3, sf::Vector2f(center.x, center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y3));
	shape.setPoint(4, sf::Vector2f(center.x + 0.95 * GAMESPACE_TO_RENDERSPACE * X2, center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y2));
	shape.setPoint(5, sf::Vector2f(center.x + 0.95 * GAMESPACE_TO_RENDERSPACE * X2, center.y + 0.95 * GAMESPACE_TO_RENDERSPACE * Y1));
	sf::Color C;
	sf::Text text;
	if (tile.owner != -1) {
		text.setFont(Rendering::Renderer::font);
		text.setPosition({center.x - 40,center.y});
		text.setString(std::to_string(tile.owner) + "," + std::to_string(tile.subowner));
		text.setScale({ 1,-1 });
		this->RenderWindow::draw(text);
		C = sf::Color::Red;
	}
	else {
		C.r = 112;
		C.g = 128;
		C.b = 144;
		C.a = 255;
	}
	shape.setFillColor(sf::Color::Transparent);
	shape.setOutlineThickness(-10);
	shape.setOutlineColor(C);
	this->RenderWindow::draw(shape);
}