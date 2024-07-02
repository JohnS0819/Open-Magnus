#include "pch.h"
#include "gtest/gtest.h"
#include "Grid.hpp"
#include "KD_Tree.hpp"
#include <random>


TEST(KD_TREE_TESTS, query_tests) {
	using namespace Geometry;
	std::vector<coordinate> coords;
    std::vector<Circle> regions;
    std::random_device rd;
    std::mt19937 gen(rd());

    float lowerBound = -1000.0f;
    float upperBound = 1000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.0001f, 50.0f);
    for (int i = 0; i < 10000; ++i) {
        coords.push_back({ dis(gen),dis(gen) });
    }
    for (int i = 0; i < 1000; ++i) {
        Circle C;
        C.radius_ = rad(gen);
        C.center_ = { dis(gen),dis(gen) };
        regions.push_back(C);
    }
    kdt::KDTree tree(coords);
    for (auto& reg : regions) {
        auto results = tree.radiusSearch(reg.center_, reg.radius_);
        std::sort(results.begin(), results.end());
        std::vector<int> correct;
        for (int i = 0; i < coords.size(); ++i) {
            if (reg.center_.distance_to(coords[i]) < reg.radius_) {
                correct.push_back(i);
            }
        }
        std::sort(correct.begin(), correct.end());
        EXPECT_TRUE(correct == results);
    }






}


