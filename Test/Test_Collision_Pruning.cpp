#include "pch.h"
#include "Collision_Pruning.hpp"
#include <random>
#include <unordered_set>

#define iterations 100

TEST(COLLISION_PRUNING_TESTS, Insertion_querying_test) {

	int minX = -100000;
	int maxX = 100000;
	int minY = minX;
	int maxY = maxX;


	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> distX(minX, maxX);
	std::uniform_int_distribution<int> distY(minY, maxY);
	std::uniform_real_distribution <float > scale(0.0f, 1.0f);
	std::vector<Collider::rectangle> data;
	int id = 0;
	Collider N;
	for (int i = 0; i < 100; ++i) {
		Collider::rectangle rect;
		do {
			rect.LX = distX(gen);
			rect.LY = distY(gen);
			rect.UX = distX(gen);
			rect.UY = distY(gen);

			// Ensure that LX is smaller than UX, and LY is smaller than UY
			if (rect.LX > rect.UX) {
				std::swap(rect.LX, rect.UX);
			}
			if (rect.LY > rect.UY) {
				std::swap(rect.LY, rect.UY);
			}
			float deltax = rect.UX - rect.LX;
			float deltay = rect.UY - rect.LY;
			deltax *= scale(gen) * scale(gen) * scale(gen);
			deltay *= scale(gen) * scale(gen) * scale(gen);
			rect.UX = rect.LX + int(deltax);
			rect.UY = rect.LY + int(deltay);
		} while (rect.LX == rect.UX || rect.LY == rect.UY); // Continue if the rectangle has zero area
		++id;
		data.push_back(rect);
		N.insert(rect);

	}
	N.intersect();
	for (int j = 0; j < id; ++j) {
		auto &returned = N.output[j];
		std::unordered_set<int> correct;
		auto baba = data[j];
		for (int i = j; i < id; ++i) {
			if (i == j) {
				continue;
			}
			auto compare = data[i];
			if (compare.UX <= baba.LX || // rect1 is to the left of rect2
				compare.LX >= baba.UX || // rect1 is to the right of rect2
				compare.UY <= baba.LY || // rect1 is below rect2
				compare.LY >= baba.UY) { // rect1 is above rect2
				continue;
			}
			correct.insert(i);


		}
		EXPECT_TRUE(returned == correct);

	}

}