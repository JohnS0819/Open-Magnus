#ifndef TILE_HPP
#define TILE_HPP


#include "Instruction.hpp"
#include "Transforms.hpp"
#include "Molecule.hpp"
#include "Grid.hpp"
#include "KD_Tree.hpp"
#include "Renderer.hpp"

template<>
struct std::hash<std::pair<int, int>> {
	std::size_t operator()(const std::pair<int, int>& k) const {
		std::hash<int> hasher;
		return hasher(k.first) ^ (hasher(k.second) >> 1);
	}

};

class GridTile : public Geometry::coordinate {
private:
	friend class TileManager;
public:
	struct spot {
		Geometry::grid_coordinate location;
		int owner;
		int subowner;

		bool is_empty() {
			return owner == -1;
		}
	};
	std::vector<spot> spots;

	enum Type {
		disconnector = 0,
		connecter = 1
	};
	Type type;

	void clear() {
		for (auto& i : spots) {
			i.owner = -1;
		}
	}


};


template<>
void Rendering::Render_Window::draw(const GridTile::spot&);

class TileManager {
protected:
	class TESTING_ACCESSOR;

	friend class TESTING_ACCESSOR;

	std::vector<GridTile> tiles_;

	kdt::KDTree<GridTile> kdTree;

	std::vector<int> connectivityMap;

	TileManager();

	friend class ConnectionManager;

	friend class Molecule;

	friend class MoleculeManager;

	//size of connected region {owner, subowner}
	std::vector<std::vector<int>> partitionSizes;

	//{owner, subowner}
	std::vector<std::vector<std::pair<int, int>>> partitions;

	//weakly connected partition indices
	std::vector<std::vector<int>> tasks;

	//map that defines which partition a molecule is cannibalized by
	std::vector<int> moleculeOwners;

	//map that defines which molecule a partition cannibalizes
	std::vector<int> partitionLinks;


	//list of molecule ids that contains only singlet partitions, ie ones which
	//correspond to disconnected nodes in the Connectivity map
	std::vector<int> TrivialTasks;
public:

	int getNumberOfTasks() const;

	int getNumberOfTrivialTasks() const;

	/*
	!@brief Creates the list of partitions, each partition contains a list of all subgraphs that make it up
	*/
	void generatePartitions();

	/*
	!@brief creates connectivity map, this maps the index of a molecule to the id of a task.
	!each task contains a disjoint set of partitions that need to be formed
	*/
	void generateConnectivityMap();

	/*
	!@brief creates the tasks. assumes that both the generateTaskPartitions() and generatePartitions() methods have already returned
	*/
	void generateTasks();

	/*
	* !@brief for a given task id, configure tasks by finding good links for every partition within the task,
	*/
	void configureTask(int id);

	/*'
	* !@brief for a given trivial task id, determine which component of the original molecule will be reused, all others will be need
	to be created
	*/

	const std::vector<int>& getTrivialTasks() const;

	void configureTrivialTask(int id);

	void reset();

	void query(Molecule&, int moleculeIndex);

	static TileManager& instance();

	void build();

	void insert(const GridTile&);
};

#endif