#ifndef TRANSFORMS_HPP
#define TRANSFORMS_HPP

#include "Grid.hpp"

namespace Geometry {
	//Transform representing a rigid motion for the grid coordinates
	struct Coordinate_Transform {
		//first matrix row
		grid_coordinate x_mat;
		//second matrix row
		grid_coordinate y_mat;
		grid_coordinate delta;


		grid_coordinate operator()(const grid_coordinate&) const;

		grid_coordinate ApplyRotation(const grid_coordinate&) const;

		Coordinate_Transform inverse() const;

		Coordinate_Transform();

		Coordinate_Transform operator*(const Coordinate_Transform& other) const;

		void Rotate();

		void counterRotate();

		void translate(const grid_coordinate&);

	};




	//Transform representing a rotation THEN a translation
	struct Rigid_Transform {
		Rigid_Transform(const Coordinate_Transform&);

		Rigid_Transform();

		coordinate Translation;
		coordinate Rotation;

		void Apply(coordinate& target) const;


		// in place transform the current transformation by another transformation (ie other * this) 
		void Commpose(const Rigid_Transform& other);

		Rigid_Transform operator*(const Rigid_Transform& other) const;

		coordinate operator*(const coordinate& other) const;
	};

	


	grid_coordinate closestHexagon(const coordinate&);





}






#endif