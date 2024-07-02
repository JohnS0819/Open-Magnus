#include "R_Tree.hpp"



std::vector<Mechanism*> R_Tree::query(const box& query) const {
	std::vector<Mechanism*> output;

	std::vector<CastingIntermediate>& casted = reinterpret_cast<std::vector<CastingIntermediate>&>(output);
	static_assert(sizeof(CastingIntermediate) == sizeof(Mechanism*));
	tree_.query(boost::geometry::index::intersects(query), std::back_inserter(casted));
	return output;
}

R_Tree::R_Tree(const std::vector<RectPair>& data) : tree_(data.begin(),data.end()) {}


boost::geometry::model::box<boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>> convertToBoostFormat(const Geometry::Rectangle& rect) {
	typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> point;
	boost::geometry::model::box<point> output(point(rect.LX, rect.LY), point(rect.RX, rect.UY));
	return output;
}