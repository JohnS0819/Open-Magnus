#include "pch.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <gtest/gtest.h>
// to store queries results
#include <vector>

// just for output
#include <iostream>
#include <boost/foreach.hpp>
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::point<float, 2, bg::cs::cartesian> point;


//struct casting {
//    typedef bg::model::box<point> box;
//    typedef std::pair<box, unsigned> value;
//
//    casting() = default;
//    int value_;
//
//    casting(const value& b) : value_(b.second) {}
//
//    //casting(value&& b) : value_(b.second) {}
//};
//
//
//
//TEST(badTest, goodTest) {
//    typedef bg::model::box<point> box;
//    typedef std::pair<box, unsigned> value;
//
//    // create the rtree using default constructor
//    bgi::rtree< value, bgi::quadratic<16> > rtree;
//    std::vector<box> boxes;
//    box b1(point(0, 0), point(2, 2));
//    box b2(point(1, 1), point(3, 3));
//    box b3(point(1.6, 1.6), point(2, 2));
//    boxes.push_back(b1);
//    boxes.push_back(b2);
//    boxes.push_back(b3);
//    for (int i = 0; i < boxes.size(); ++i) {
//        rtree.insert(std::make_pair(boxes[i], i));
//    }
//    box query(point(0.5, 0.5), point(1.5, 1.5));
//    std::vector<casting> results; 
//    auto testingfunctor = [&]() {return std::back_insert_iterator(results); };
//    rtree.query(bgi::intersects(query), std::back_insert_iterator(results));
//    for (auto& i : results) {
//        EXPECT_EQ(i.value_, 1);
//    }
//    boost::geometry::index::pack
//
//
//}
