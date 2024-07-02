#ifndef COLLIDABLE_HPP
#define COLLIDABLE_HPP
#include "Motion.hpp"
#include <iostream>


class Coarse_Collidable : public Geometry::Circle{
public:
	using Geometry::Circle::center_;
	using Geometry::Circle::radius_;

	bool collide(const Coarse_Collidable& target, const Motion::Stationary&, const Motion::Stationary&) const;

	bool collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::CCW_Rotation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::CW_Rotation& M1, const Motion::CW_Rotation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::CW_Rotation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::Translation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::Translation& M1, const Motion::CCW_Rotation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::CW_Rotation &M1, const Motion::Translation& M2) const;

	bool collide(const Coarse_Collidable& target, const Motion::Translation &M1, const Motion::CW_Rotation& M2) const;
};


#endif // !COLLIDABLE_HPP

