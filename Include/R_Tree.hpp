#ifndef R_TREE_HPP
#define R_TREE_HPP
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>
#include "Grid.hpp"


class Mechanism;

boost::geometry::model::box<boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>> convertToBoostFormat(const Geometry::Rectangle&);









class R_Tree {
public:
	typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> point;
	typedef boost::geometry::model::box<point> box;
	typedef std::pair<box, Mechanism*> RectPair;
	typedef boost::geometry::index::rtree<RectPair, boost::geometry::index::quadratic<16>> rtree;

protected:
	


	struct CastingIntermediate {
		Mechanism* pointer;

		CastingIntermediate() = default;

		CastingIntermediate(const std::pair<box, Mechanism*>& val) : pointer(val.second) {}

		CastingIntermediate(std::pair<box, Mechanism*>&& val) : pointer(std::move(val.second)) {}


	};


	
	rtree tree_;
public:
	std::vector<Mechanism*> query(const box&) const;

	R_Tree(const std::vector<RectPair>&);

};



#endif