#include "Graph.hpp"
#include <map>
#include <unordered_set>
#include <unordered_map>


Graph::Collision_Hierarchy::Collision_Hierarchy(Geometry_Graph &A) : coord_map(std::move(A.coord_map)) {
	nodes.push_back(std::move(A.nodes));
}

Graph::Collision_Hierarchy::Collision_Hierarchy(Geometry_Graph&& A) : coord_map(std::move(A.coord_map)) {
	nodes.push_back(std::move(A.nodes));
}

void Graph::markComponent(std::vector<Collision_Leaf>& graph, int index, const int& marker) {
	if (graph[index].children[0] != -1) {
		return;
	}
	graph[index].children[0] = marker;
	for (int i = 1; i < 7; ++i) {
		if (graph[index].children[i] == -1) {
			continue;
		}
		markComponent(graph, graph[index].children[i], marker);
	}
}

const std::vector<std::vector<Graph::Collision_Leaf>>& Graph::Collision_Hierarchy::getNodes() const {
	return nodes;
}

bool Graph::Collision_Hierarchy::query(const Geometry::grid_coordinate& point) const {
	return coord_map[0].find(point) != coord_map[0].end();

}

Graph::Collision_Hierarchy::Collision_Hierarchy() {

}

Coarse_Collidable Graph::Collision_Hierarchy::getBoundingCircle() const {
	return this->nodes.back()[this->coord_map.back().begin()->second].collision;
}

std::pair<Geometry::grid_coordinate, int> Graph::Collision_Hierarchy::findParent(const Geometry::grid_coordinate& initial) {
	int new_i = initial.i - 2 * initial.j;
	int new_j = 2 * initial.i + 3 * initial.j;

	Geometry::grid_coordinate new_coordinate = { new_i / 7, new_j / 7 };
	Geometry::grid_coordinate modulo = { new_i % 7, new_j % 7 };
	if (modulo.i < -3) {
		--new_coordinate.i;
		modulo.i += 7;
	}
	else if (modulo.i > 3) {
		++new_coordinate.i;
		modulo.i -= 7;
	}
	if (modulo.j < -3) {
		--new_coordinate.j;
		modulo.j += 7;
	}
	else if (modulo.j > 3) {
		++new_coordinate.j;
		modulo.j -= 7;
	}
	return { new_coordinate,modulo.i + modulo.j + 3 };
}


Coarse_Collidable Graph::Collision_Hierarchy::updateCollider(const int& level, const int& index) {
	Coarse_Collidable output;
	output.center_ = { 0,0 };
	output.radius_ = -1;
	Geometry::coordinate average = { 0,0 };
	int n = 0;
	for (auto& j : nodes[level][index].children) {
		if (j == -1) {
			continue;
		}
		++n;
		average.move_to(nodes[level - 1][j].collision.center_);

	}
	if (n == 0) return output;
	average *= 1.0f / (float(n));
	float R = -10.0f;
	int p1 = -1;
	for (auto& j : nodes[level][index].children) {
		if (j == -1) {
			continue;
		}

		if ((average.distance_to(nodes[level - 1][j].collision.center_) + nodes[level - 1][j].collision.radius_) > R) {
			R = average.distance_to(nodes[level - 1][j].collision.center_) + nodes[level - 1][j].collision.radius_;
			p1 = j;
		}
	}
	if (R == nodes[level - 1][p1].collision.radius_) {
		output.radius_ = R;
		output.center_ = average;
		return output;
	}
	Geometry::coordinate P1 = ((nodes[level - 1][p1].collision.center_ - average) * (R / (R - nodes[level - 1][p1].collision.radius_))) + average;
	int p2 = -1;
	float D = 0.0f;
	float D1 = 0.0f;
	for (auto& j : nodes[level][index].children) {
		if (j == -1) {
			continue;
		}

		if (P1.distance_to(nodes[level - 1][j].collision.center_) + nodes[level - 1][j].collision.radius_ > D) {
			D = P1.distance_to(nodes[level - 1][j].collision.center_) + nodes[level - 1][j].collision.radius_;
			D1 = P1.distance_to(nodes[level - 1][j].collision.center_);
			p2 = j;
		}
	}
	Geometry::coordinate P2 = ((nodes[level - 1][p2].collision.center_ - P1) * ((D) / D1)) + P1 - average;
	P1 -= average;

	float t = (D * D) / (2.0f * (R * R - P1.x * P2.x - P1.y * P2.y));
	P2 += average;

	average += (P1 * -t) + P1;
	for (auto& j : nodes[level][index].children) {
		if (j == -1) {
			continue;
		}
		output.radius_ = std::max(average.distance_to(nodes[level - 1][j].collision.center_) + nodes[level - 1][j].collision.radius_, output.radius_);
	}
	output.center_ = average;
	return output;
}

void Graph::Collision_Hierarchy::subdivide() {
	coord_map.push_back({});
	coord_map.back().reserve(coord_map[coord_map.size() - 1].size() / 2);
	auto& last_map = coord_map[coord_map.size() - 2];
	auto& M = coord_map.back();
	nodes.push_back({});
	nodes.back().reserve(coord_map[coord_map.size() - 2].size() / 2);
	int size = nodes.size() - 1;
	int node_number = 0;
	for (auto& i : last_map) {
		auto parent = findParent(i.first);

		if (M.find(parent.first) == M.end()) {
			M[parent.first] = node_number;
			nodes[size].push_back(parent.first);
			++node_number;
		}
		nodes[size][M[parent.first]].children[parent.second] = i.second;

	}
	for (int i = 0; i < node_number; ++i) {
		nodes[size][i].collision = updateCollider(size, i);
		/*Geometry::coordinate average = { 0,0 };
		int n = 0;
		for (auto& j : nodes[size][i].children) {
			if (j == -1) {
				continue;
			}
			++n;
			average.move_to(nodes[size-1][j].collision.center_);

		}
		average *= 1.0f / (float(n));
		float R = -10.0f;
		int p1 = -1;
		for (auto& j : nodes[size][i].children) {
			if (j == -1) {
				continue;
			}

			if ((average.distance_to(nodes[size - 1][j].collision.center_) + nodes[size - 1][j].collision.radius_) > R) {
				R = average.distance_to(nodes[size - 1][j].collision.center_) + nodes[size - 1][j].collision.radius_;
				p1 = j;
			}
		}
		if (R == nodes[size - 1][p1].collision.radius_) {
			nodes[size][i].collision.radius_ = R;
			nodes[size][i].collision.center_ = average;
			continue;
		}
		Geometry::coordinate P1 = ((nodes[size-1][p1].collision.center_ - average) * (R / (R - nodes[size-1][p1].collision.radius_))) + average;
		int p2 = -1;
		float D = 0.0f;
		float D1 = 0.0f;
		for (auto& j : nodes[size][i].children) {
			if (j == -1) {
				continue;
			}

			if (P1.distance_to(nodes[size - 1][j].collision.center_) + nodes[size - 1][j].collision.radius_ > D) {
				D = P1.distance_to(nodes[size - 1][j].collision.center_) + nodes[size - 1][j].collision.radius_;
				D1 = P1.distance_to(nodes[size - 1][j].collision.center_);
				p2 = j;
			}
		}
		Geometry::coordinate P2 = ((nodes[size - 1][p2].collision.center_ - P1) * ((D) / D1)) + P1 - average;
		P1 -= average;

		float t = (D * D) / (2.0f * (R * R - P1.x * P2.x - P1.y * P2.y));
		P2 += average;

		average += (P1 * -t) + P1;
		for (auto& j : nodes[size][i].children) {
			if (j == -1) {
				continue;
			}
			nodes[size][i].collision.radius_ = std::max(average.distance_to(nodes[size-1][j].collision.center_) + nodes[size - 1][j].collision.radius_, nodes[size][i].collision.radius_);
		}
		nodes[size][i].collision.center_ = average;*/
	}



}