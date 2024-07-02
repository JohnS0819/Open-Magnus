#ifndef MOTION_HPP
#define MOTION_HPP
#include "Grid.hpp"
#include "Collision_Pruning.hpp"
#include "Transforms.hpp"

class Molecule;
namespace Motion {
	class Motion_Type {
	public:
		enum ids_ {
			nothing = 0,
			stationary = 1,
			ccw_rot = 2,
			cw_rot = 3,
			trans = 4
		};
	protected:
		
		//cant be const for union shenanigans, so just don't ever modify
		ids_ id;

		Motion_Type(const ids_& num);

	public:
		friend class ::Molecule;
		
		ids_ getId() const;

		virtual Collider::rectangle Generate_Bounding_Box(const Geometry::Circle& obj) const = 0;

		virtual Geometry::Rigid_Transform getTransform(const double& interpolationFactor) const = 0;
	};

	class Stationary : public Motion_Type {
	private:
		using Motion_Type::ids_;
		
		//doesn't do anything, only there for data alignment
		Geometry::coordinate data_;
	
	public:
		friend class ::Molecule;

		Stationary();

		virtual Geometry::Rigid_Transform getTransform(const double& interpolationFactor) const;

		virtual Collider::rectangle Generate_Bounding_Box(const Geometry::Circle& obj) const;
	};

	class CW_Rotation : public Motion_Type {
	private:

		using Motion_Type::ids_;

	public:
		friend class ::Molecule;

		CW_Rotation();

		CW_Rotation(const Geometry::coordinate&);

		Geometry::coordinate pivot_;

		virtual Geometry::Rigid_Transform getTransform(const double& interpolationFactor) const;

		virtual Collider::rectangle Generate_Bounding_Box(const Geometry::Circle& obj) const;

		bool operator==(const CW_Rotation&) const;
	};
	class CCW_Rotation : public Motion_Type {
	private:

		using Motion_Type::ids_;

	public:
		friend class ::Molecule;

		CCW_Rotation();

		CCW_Rotation(const Geometry::coordinate&);

		Geometry::coordinate pivot_;
		
		virtual Geometry::Rigid_Transform getTransform(const double& interpolationFactor) const;

		virtual Collider::rectangle Generate_Bounding_Box(const Geometry::Circle& obj) const;

		bool operator==(const CCW_Rotation&) const;
	};

	class Translation : public Motion_Type {
	private:

		using Motion_Type::ids_;

	public:
		
		friend class ::Molecule;

		Translation();

		Translation(const Geometry::coordinate&);

		Geometry::coordinate delta_;

		virtual Geometry::Rigid_Transform getTransform(const double& interpolationFactor) const;

		virtual Collider::rectangle Generate_Bounding_Box(const Geometry::Circle& obj) const;

		bool operator==(const Translation&) const;

	};
};







#endif // !MOTION_HPP
