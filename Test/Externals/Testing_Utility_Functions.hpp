#ifndef TESTING_UTILITIES
#define TESTING_UTILITIES
#include "Graph.hpp"
#include "Molecule.hpp"
#include "Connections.hpp"
#include "Tile.hpp"
#include "Utils.hpp"
#include <random>


class TESTING_MOLECULEMANGAER_OVERRIDE {
public:
	static void setNumber(int N) {
		MoleculeManager::instance().molecules_.resize(N);
	}

	static Graph::Collision_Hierarchy& getHierarchy(Molecule& M) {
		return MoleculeManager::getHierarchy(M);
	}

	static std::vector<int>& getComponentMap(Molecule& M) {
		return MoleculeManager::getComponentMap(M);
	}

	static std::vector<std::vector<int>>& getAvailable(Molecule & M) {
		return MoleculeManager::getAvailable(M);
	}

	static Geometry::Coordinate_Transform& getTransform(Molecule& M) {
		return MoleculeManager::getTransform(M);
	}


};



class TileManager::TESTING_ACCESSOR {
public:
	static std::vector<int>& getconnectivityMap(TileManager&);

	static std::vector<std::vector<int>>& getpartitionSizes(TileManager&);

	static  std::vector<std::vector<std::pair<int, int>>>& getpartitions(TileManager&);

	static std::vector<std::vector<int>> gettasks(TileManager&);
	
	static std::vector<int>& getmoleculeOwners(TileManager&);

	static std::vector<int>& getpartitionLinks(TileManager&);

	static std::vector<int>& getTrivialTasks(TileManager&);
};


class MockFixture : public TileManager, public ConnectionManager {
protected:
	TileManager* theft;
	char* data;
	bool Instance_Stolen;

    void StealInstance();

    void returnInstance();

	using TileManager::instance;

	static TESTING_ACCESSOR ACCESSOR;
public:
    MockFixture();

    ~MockFixture();
};


Graph::Geometry_Graph generate_Graph_in_Boundary(const int& minimum, const int& maximum, const int& max_nodes, const std::unordered_set<Geometry::grid_coordinate>& occupied, const Geometry::Coordinate_Transform& T, const double& edge_density);

namespace UTILITY {
	std::vector<Graph::Collision_Leaf> generate_nodes(int n);
}

#endif // !TESTING_UTILITIES