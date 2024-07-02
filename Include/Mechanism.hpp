#ifndef MECHANISM_HPP
#define MECHANISM_HPP
#include "Instruction.hpp"
#include "Collidable.hpp"
#include "Transforms.hpp"
#include "Grid.hpp"
#include "KD_Tree.hpp"
#include "R_Tree.hpp"
#include <memory>
#include <array>


class Molecule;

class Mechanism : public Coarse_Collidable, public Geometry::grid_coordinate{
public:
	
	
	class Head : public Geometry::grid_coordinate{
	public:
		enum HeadState {
			open = 0,
			closed = 1
		};
	public:
		HeadState state_;
		
		bool queryable_;

		bool attached_ = false; 

		const Instructions::GenericInstruction* const * ptr_;


	public:
		const Instructions::GenericInstruction* getCurrentInstruction() const;

		Geometry::grid_coordinate getMotionData() const;

		void updatePosition();

		void updateQueryable();

		bool isAttached() const;

		bool attach(Molecule& M);

		bool isQueryable() const;



	};

private:
	friend class MechanismManager;

	int tape_position;

	Geometry::grid_coordinate position = Geometry::grid_coordinate(0,0);

	

	Motion::Motion_Type::ids_ motionType = Motion::Motion_Type::ids_::ccw_rot;
public:
	void updateOrientation();
	
	std::vector<Head*> heads_;
	
	const Instructions::GenericInstruction *currentInstruction;

public:
	Instructions::Instruction_Tape tape;

	bool intersect(Molecule&);

	

	//const std::vector<std::unique_ptr<Head>>& getHeads() const;

	virtual void read();

	virtual void updateHeads();

	virtual Geometry::Rectangle getBoundingBox() const;

	Geometry::grid_coordinate getPosition() const;

	Motion::Motion_Type::ids_ getMotionType() const;

	Mechanism() = default;

	Mechanism(const Mechanism&) = default;

	Mechanism(std::vector<std::shared_ptr<Instructions::GenericInstruction>>&& other);
	
};


class MechanismManager {
protected:
	std::vector<Mechanism*> mechanisms_;

	std::unique_ptr<R_Tree> tree_;


public:
	void buildTree();

	std::vector<Mechanism*> query(const Molecule&) const;

	void insert(const Mechanism&);

	void insert(Mechanism*);


};



#endif // !MECHANISM_HPP
