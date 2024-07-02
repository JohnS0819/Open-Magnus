#ifndef MOLECULE_HPP
#define MOLECULE_HPP

#include "Graph.hpp"
#include "Transforms.hpp"
#include <SFML/Graphics.hpp>
#include "Asynchronous_Stack.hpp"

class Mechanism;

class Molecule;

namespace Rendering {

//length from center of unit hexagon to vertex
#define HEXAGONAL_LENGTH 0.57735026919f
//hexagonal radius * (golden ratio - 1)
#define ATOMIC_RADIUS 0.356822089773f
#define NUMBER_OF_CIRCLE_POINTS 12



	struct CIRCLE_PNTS {
	private:
		//C++ doesn't have constexpr trig functions
		constexpr static double sin(float value) {
			double output = 0.0;
			double initial = value;
			double power = value;
			uint64_t denom = 1;
			for (int i = 1; i < 15; ++i) {
				if ((i % 2) == 0) {
					denom *= (i + 1);
					power *= initial;
					continue;
				}
				if ((i % 4) == 1) {
					double casted = double(denom);
					output += power / denom;
					denom *= (i + 1);
					power *= initial;
				}
				else {
					double casted = double(denom);
					output -= power / denom;
					denom *= (i + 1);
					power *= initial;
				}

			}
			return output;
		}
	public:
		constexpr CIRCLE_PNTS() : points() {
			constexpr double two_pi = 6.28318530717958647692528676;
			constexpr double increment = two_pi / NUMBER_OF_CIRCLE_POINTS;
			constexpr double pi_over_two = 1.570796326794896619231321691;
			double initial = 0.0;
			for (auto i = 0; i < NUMBER_OF_CIRCLE_POINTS; ++i) {
				if (initial >= 3.14159265359) {
					points[2 * i + 1] =  Geometry::SPACE_TO_PIXELS * ATOMIC_RADIUS * -CIRCLE_PNTS::sin(initial - two_pi * 0.5);
					points[2 * i] = Geometry::SPACE_TO_PIXELS * ATOMIC_RADIUS * CIRCLE_PNTS::sin(initial - two_pi + pi_over_two);
				}
				else {
					points[2 * i + 1] = Geometry::SPACE_TO_PIXELS * ATOMIC_RADIUS * CIRCLE_PNTS::sin(initial);
					points[2 * i] = Geometry::SPACE_TO_PIXELS * ATOMIC_RADIUS * -CIRCLE_PNTS::sin(initial - pi_over_two);
				}
				initial += increment;
			}
		}
		float points[2 * NUMBER_OF_CIRCLE_POINTS];

	};

	class Atom_Primitive {
	private:
		static constexpr CIRCLE_PNTS COORDS_ = CIRCLE_PNTS();
	public:
		sf::Vertex vertices[3 * NUMBER_OF_CIRCLE_POINTS];

		Atom_Primitive(const Geometry::grid_coordinate& coord);

		Atom_Primitive(const Atom_Primitive&);

		Atom_Primitive();
	};



	class Molecule_Primitive {
	private:
		
	public:
		const Molecule* ptr;

		Geometry::Rigid_Transform Orientation;
		
		sf::Transform SFTRANSFORM;

		std::vector<Atom_Primitive> atoms;

		sf::VertexArray connections;

		std::vector<sf::CircleShape> collisions;

		Molecule_Primitive();

		Molecule_Primitive(const Molecule_Primitive&) = default;


	};

}

class GridTile;

class Molecule {

private:
	std::vector<std::vector<int>> available_;

	int start_of_update_;

	Geometry::grid_coordinate motion_data_;

protected:


	union MotionUnion
	{
		Motion::CCW_Rotation ccw_state;
		Motion::CW_Rotation cw_state;
		Motion::Stationary stationary_state;
		Motion::Translation translation_state;

		MotionUnion() {
			static_assert(sizeof(MotionUnion) == sizeof(Motion::CCW_Rotation));
			static_assert(sizeof(Motion::CCW_Rotation) == sizeof(Motion::CW_Rotation));
			static_assert(sizeof(Motion::CW_Rotation) == sizeof(Motion::Translation));
			static_assert(sizeof(Motion::CW_Rotation) == sizeof(Motion::Stationary));

			static_assert(offsetof(Motion::Motion_Type, id) == offsetof(Motion::CCW_Rotation, id));
			static_assert(offsetof(Motion::CCW_Rotation, pivot_) == offsetof(Motion::CW_Rotation, pivot_));
			static_assert(offsetof(Motion::CCW_Rotation, pivot_) == offsetof(Motion::Translation, delta_));
		}
	};
	friend class MoleculeManager;
	
	friend class ConnectionManager;

	int query_internal(const Geometry::grid_coordinate&) const;

	const MotionUnion* motionState;

	Geometry::Rigid_Transform Orientation;

	Geometry::Coordinate_Transform coordTransform;

	Geometry::Coordinate_Transform inverse_map;

	Graph::Collision_Hierarchy Colliders;

	std::unordered_map<int,int*> connectionCallback;

	bool ConnectionsUpdated = false;

	std::vector<int> componentMap;

	


	/*
	!@brief updates the collision primitives in the current subdivision level then bubble those up to the next level
	*/
	void bubbleUpdate(int level,std::unordered_set<int>& updates, std::unordered_set<int>& buffer_);

	void resizeHierarchy(int newSize);


	void rotate(const Geometry::grid_coordinate&);

	void counterRotate(const Geometry::grid_coordinate&);

	void translate(const Geometry::grid_coordinate&);

	/*bool setMotion(const Motion::CCW_Rotation&);

	bool setMotion(const Motion::Translation&);

	bool setMotion(const Motion::Stationary&);

	bool setMotion(const Motion::CW_Rotation&);*/

	void recycle();

public:
	/*
	!@brief resets Metadata
	*/
	void reset();

	Molecule(Graph::Collision_Hierarchy &&, Geometry::Coordinate_Transform);

	Molecule(Graph::Collision_Hierarchy&&);

	Molecule();

	void updateConnection(GridTile&, int);

	void updateOrientation();

	bool setMotion(const Motion::Motion_Type*, const Geometry::grid_coordinate&);

	//void setMotionData(const Geometry::grid_coordinate&);

	void callbackConnections(int id);

	Geometry::Circle getBoundingCircle() const;

	Geometry::Rectangle getBoundingBox() const;

	bool query(const Geometry::grid_coordinate&) const;	

	void clearMotion();

	sf::Transform getRenderTransform() const;

	Collider::rectangle GenerateCollisionBox() const;

};


class TESTING_MOLECULEMANGAER_OVERRIDE;

class MoleculeManager {
private:
	struct MoleculePtr {
		Molecule* ptr;
		MoleculePtr() : ptr(nullptr) {}
		MoleculePtr(Molecule* const& value) : ptr(value) {}
		MoleculePtr(const MoleculePtr&) = default;
		MoleculePtr(MoleculePtr&& other) : ptr(other.ptr) { other.ptr = nullptr;}

		~MoleculePtr() {
			if (ptr != nullptr) {
				delete ptr;
			}
		}

		void operator=(const MoleculePtr& M) {
			ptr = M.ptr;
		}
	};

	AsynchronousStack<int> indicesToRemove;
	AsynchronousStack<MoleculePtr> popper;
	AsynchronousStack<MoleculePtr> pusher;
	


protected:
	MoleculeManager();
	static Geometry::Coordinate_Transform& getTransform(Molecule&);

	static std::vector<std::vector<int>>& getAvailable(Molecule&);

	static Graph::Collision_Hierarchy& getHierarchy(Molecule&);

	static std::vector<int>& getComponentMap(Molecule&);

	std::vector<Molecule*> molecules_;
	
	friend class TESTING_MOLECULEMANGAER_OVERRIDE;

	Rendering::Molecule_Primitive generateInternal(const Molecule*) const;

	void trimMoleculeList();
public:

	void TESTING();

	std::vector<Rendering::Molecule_Primitive> generateRenderables() const;

	void insert(const Molecule& M);

	void insert(Molecule *M);

	void markForDestruction(int id);

	Molecule* getNewMolecule();

	void delayDestruction(int id);

	static MoleculeManager& instance();

	Molecule* query(const Geometry::grid_coordinate&) const;

	int number() const;

	std::vector<Molecule*>& TESTUTIL();



};


#endif // !MOLECULE_HPP

