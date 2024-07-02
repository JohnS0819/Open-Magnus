#ifndef GRAPH_HPP
#define GRAPH_HPP
#include "Collidable.hpp"
#include <array>
#include <unordered_map>
#include <utility>

class ConnectionManager;
class MoleculeManager;
namespace Graph {

	class Hexnode {
	public:
		std::array<int, 7> children;
		Hexnode() {
			children.fill(-1);
		}
	};

	class Collision_Leaf {
	public:
		std::array<int, 7> children;
		Geometry::grid_coordinate coord;
		Coarse_Collidable collision;
		Collision_Leaf() {
			children.fill(-1);
		}
		Collision_Leaf(const Geometry::grid_coordinate& g) : coord(g) {
			children.fill(-1);
		}
	};

	void markComponent(std::vector<Collision_Leaf>& graph, int index, const int& marker);



	class Geometry_Graph {
	public:
		std::vector<std::unordered_map<Geometry::grid_coordinate, int>> coord_map;
		std::vector<Collision_Leaf> nodes;

	};

	class Collision_Hierarchy {
	private:
		friend class Molecule;

		friend class ConnectionManager;

		friend class MoleculeManager;

		std::vector<std::vector<Collision_Leaf>> nodes;

		std::vector<std::unordered_map<Geometry::grid_coordinate, int>> coord_map;


		Coarse_Collidable updateCollider(const int& level, const int& index);

	public:
		static std::pair<Geometry::grid_coordinate, int> findParent(const Geometry::grid_coordinate& initial);

		void subdivide();

		const std::vector<std::vector<Collision_Leaf>>& getNodes() const;

		Coarse_Collidable getBoundingCircle() const;

		bool query(const Geometry::grid_coordinate&) const;

		Collision_Hierarchy();

		Collision_Hierarchy(Geometry_Graph& A);
		Collision_Hierarchy(Geometry_Graph&& A);


	};



}















#endif