#include "pch.h"
#include "Motion.hpp"
#include <random>


#define iterations 1000

TEST(MOTION_TESTS, Test_CCW_Bounding_Box) {

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution <float > dist(-1000.0f, 1000.0f);
	std::uniform_real<float> rad(0.01f, 1000.0f);
	for (int i = 0; i < iterations; ++i) {
		Geometry::Circle C;
		Motion::CCW_Rotation M;
		M.pivot_.x = dist(gen);
		M.pivot_.y = dist(gen);
		C.center_.x = dist(gen);
		C.center_.y = dist(gen);
		while (C.center_.x == M.pivot_.x && C.center_.y == M.pivot_.y) {
			C.center_.x = dist(gen);
			C.center_.y = dist(gen);
		}
		C.radius_ = rad(gen);
		//C.center_.x = M.pivot_.x + -1452.5107421875;
		//C.center_.y = M.pivot_.y + 1165.91796875;

		Geometry::Circle C2;
		C2.center_ = C.center_ - M.pivot_;
		C2.radius_ = C.radius_;

		float min_x, max_x, min_y, max_y;

		min_x = C2.center_.x;
		min_y = C2.center_.y;
		max_x = C2.center_.x;
		max_y = C2.center_.y;
		for (float j = 0.0f; j <= 1.0471975512f; j += 0.001f) {
			Geometry::coordinate C1 = C2.center_;
			C1.rotate(j);
			min_x = std::min(min_x, C1.x);
			min_y = std::min(min_y, C1.y);
			max_x = std::max(max_x, C1.x);
			max_y = std::max(max_y, C1.y);

		}
		min_x += M.pivot_.x - C.radius_;
		min_y += M.pivot_.y - C.radius_;
		max_x += M.pivot_.x + C.radius_;
		max_y += M.pivot_.y + C.radius_;

		auto R = M.Generate_Bounding_Box(C);
		float delta = fabs((C.center_.x - M.pivot_.x)) + fabs((C.center_.y - M.pivot_.y));
		EXPECT_NEAR(min_x, R.LX, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(max_x, R.UX, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(min_y, R.LY, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(max_y, R.UY, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_GE(min_x, R.LX);
		EXPECT_LE(max_x, R.UX);
		EXPECT_GE(min_y, R.LY);
		EXPECT_LE(max_y, R.UY);


	}






}

TEST(MOTION_TESTS, Test_CW_Bounding_Box) {

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution <float > dist(-1000.0f, 1000.0f);
	std::uniform_real<float> rad(0.01f, 1000.0f);
	for (int i = 0; i < iterations; ++i) {
		Geometry::Circle C;
		Motion::CW_Rotation M;
		M.pivot_.x = dist(gen);
		M.pivot_.y = dist(gen);
		C.center_.x = dist(gen);
		C.center_.y = dist(gen);
		while (C.center_.x == M.pivot_.x && C.center_.y == M.pivot_.y) {
			C.center_.x = dist(gen);
			C.center_.y = dist(gen);
		}
		C.radius_ = rad(gen);
		//C.center_.x = M.pivot_.x + -1452.5107421875;
		//C.center_.y = M.pivot_.y + 1165.91796875;

		Geometry::Circle C2;
		C2.center_ = C.center_ - M.pivot_;
		C2.radius_ = C.radius_;

		float min_x, max_x, min_y, max_y;

		min_x = C2.center_.x;
		min_y = C2.center_.y;
		max_x = C2.center_.x;
		max_y = C2.center_.y;
		for (float j = 0.0f; j >= -1.0471975512f; j -= 0.001f) {
			Geometry::coordinate C1 = C2.center_;
			C1.rotate(j);
			min_x = std::min(min_x, C1.x);
			min_y = std::min(min_y, C1.y);
			max_x = std::max(max_x, C1.x);
			max_y = std::max(max_y, C1.y);

		}
		min_x += M.pivot_.x - C.radius_;
		min_y += M.pivot_.y - C.radius_;
		max_x += M.pivot_.x + C.radius_;
		max_y += M.pivot_.y + C.radius_;

		auto R = M.Generate_Bounding_Box(C);
		float delta = fabs((C.center_.x - M.pivot_.x)) + fabs((C.center_.y - M.pivot_.y));
		EXPECT_NEAR(min_x, R.LX, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(max_x, R.UX, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(min_y, R.LY, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_NEAR(max_y, R.UY, 2.0 + 0.01f * delta) << C.center_ - M.pivot_ << std::endl;
		EXPECT_GE(min_x, R.LX);
		EXPECT_LE(max_x, R.UX);
		EXPECT_GE(min_y, R.LY);
		EXPECT_LE(max_y, R.UY);


	}
}