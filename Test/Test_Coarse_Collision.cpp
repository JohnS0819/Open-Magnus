#include "pch.h"
#include "gtest/gtest.h"
#include "Collidable.hpp"
#include <random> 

#define iterations 10000

TEST(Coarse_Collision_Tests, Basic_CCW_Collisions) {
	std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = -100000.0f;
    float upperBound = 100000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.1, 10000.0);
    std::uniform_real_distribution<float> rad2(0.1, 100000.0);
    std::uniform_real_distribution<float> phase(0, 1.0471975512);
    std::uniform_real_distribution<float> offset(0, 6.28318530718);
    int failures = 0;

	for (int i = 0; i < iterations; ++i) {
        float x = dis(gen);
        float y = dis(gen);
        float r1 = rad(gen);
        float r2 = rad(gen);
        Coarse_Collidable C1;
        Geometry::coordinate target = { x,y };
        std::uniform_real_distribution<float> D2(0,0.50f * ( r1 + r2 - 0.001f));
        float dist_at_sep1 = D2(gen);
        float dist_at_sep2 = D2(gen);
        float time_at_sep = phase(gen);
        float phase1 = offset(gen);
        float phase2 = offset(gen);
        Geometry::coordinate collision1 = { dist_at_sep1,0.0f };
        collision1.rotate(phase1);
        collision1.move_to(target);
        Geometry::coordinate collision2 = { dist_at_sep2, 0.0f};
        collision2.rotate(phase2);
        collision2.move_to(target);
        float R1 = rad2(gen);
        float R2 = rad2(gen);

        float phase3 = offset(gen);
        float phase4 = offset(gen);

        Geometry::coordinate pivot1 = { R1,0.0f };
        pivot1.rotate(phase3);
        pivot1.move_to(collision1);

        Geometry::coordinate pivot2 = { R2,0.0f };
        pivot2.rotate(phase4);
        pivot2.move_to(collision2);

        Motion::CCW_Rotation baka1;
        baka1.pivot_ = pivot1;
        Motion::CCW_Rotation baka2;
        baka2.pivot_ = pivot2;

        Geometry::coordinate center1 = collision1 - pivot1;
        center1.rotate(-time_at_sep);
        center1.move_to(pivot1);
        Coarse_Collidable Tester1;
        Tester1.center_ = center1;
        Tester1.radius_ = r1;

        Geometry::coordinate debug1  = center1 - pivot1;

        Geometry::coordinate center2 = collision2 - pivot2;
        center2.rotate(-time_at_sep);
        center2.move_to(pivot2);
        Coarse_Collidable Tester2;
        Tester2.center_ = center2;
        Tester2.radius_ = r2;
        Geometry::coordinate debug2 = center2 - pivot2;
        if (Tester1.center_.distance_to(Tester2.center_) <= (Tester1.radius_ + Tester2.radius_)) continue;

        bool result = Tester1.collide(Tester2, baka1, baka2);
        if (!result) {
            failures += 1;
        }
	}
    //expect it to catch over 99.99 % of random collisions
    EXPECT_LT(failures, 100);





}

TEST(Coarse_Collision_Tests, Basic_CW_Collisions) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = -100000.0f;
    float upperBound = 100000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.1, 10000.0);
    std::uniform_real_distribution<float> rad2(0.1, 100000.0);
    std::uniform_real_distribution<float> phase(0, 1.0471975512);
    std::uniform_real_distribution<float> offset(0, 6.28318530718);
    int failures = 0;

    for (int i = 0; i < iterations; ++i) {
        float x = dis(gen);
        float y = dis(gen);
        float r1 = rad(gen);
        float r2 = rad(gen);
        Coarse_Collidable C1;
        Geometry::coordinate target = { x,y };
        std::uniform_real_distribution<float> D2(0, 0.50f * (r1 + r2 - 0.001f));
        float dist_at_sep1 = D2(gen);
        float dist_at_sep2 = D2(gen);
        float time_at_sep = phase(gen);
        float phase1 = offset(gen);
        float phase2 = offset(gen);
        Geometry::coordinate collision1 = { dist_at_sep1,0.0f };
        collision1.rotate(phase1);
        collision1.move_to(target);
        Geometry::coordinate collision2 = { dist_at_sep2, 0.0f };
        collision2.rotate(phase2);
        collision2.move_to(target);
        float R1 = rad2(gen);
        float R2 = rad2(gen);

        float phase3 = offset(gen);
        float phase4 = offset(gen);

        Geometry::coordinate pivot1 = { R1,0.0f };
        pivot1.rotate(phase3);
        pivot1.move_to(collision1);

        Geometry::coordinate pivot2 = { R2,0.0f };
        pivot2.rotate(phase4);
        pivot2.move_to(collision2);

        Motion::CW_Rotation baka1;
        baka1.pivot_ = pivot1;
        Motion::CW_Rotation baka2;
        baka2.pivot_ = pivot2;

        Geometry::coordinate center1 = collision1 - pivot1;
        center1.rotate(time_at_sep);
        center1.move_to(pivot1);
        Coarse_Collidable Tester1;
        Tester1.center_ = center1;
        Tester1.radius_ = r1;

        Geometry::coordinate debug1 = center1 - pivot1;

        Geometry::coordinate center2 = collision2 - pivot2;
        center2.rotate(time_at_sep);
        center2.move_to(pivot2);
        Coarse_Collidable Tester2;
        Tester2.center_ = center2;
        Tester2.radius_ = r2;
        Geometry::coordinate debug2 = center2 - pivot2;
        if (Tester1.center_.distance_to(Tester2.center_) <= (Tester1.radius_ + Tester2.radius_)) continue;

        bool result = Tester1.collide(Tester2, baka1, baka2);
        if (!result) {
            failures += 1;
        }
    }
    //expect it to catch over 99.99 % of random collisions
    EXPECT_LT(failures, 100);





}


TEST(Coarse_Collision_Tests, Hard_Collisions) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = -1000.0f;
    float upperBound = 1000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.1, 1000.0);
    std::uniform_real_distribution<float> rad2(0.1, 10000.0);
    std::uniform_real_distribution<float> phase(0, 1.0471975512);
    std::uniform_real_distribution<float> offset(0, 6.28318530718);
    int failures = 0;
    int trials = 0;

    for (int i = 0; i < iterations; ++i) {
        float x = dis(gen);
        float y = dis(gen);
        float r1 = rad(gen);
        float r2 = rad(gen);
        Coarse_Collidable C1;
        Geometry::coordinate target = { x,y };
        std::uniform_real_distribution<float> D2(0, 0.495f * (r1 + r2));
        float dist_at_sep1 = D2(gen);
        float dist_at_sep2 = D2(gen);
        float time_at_sep = phase(gen);
        float phase1 = offset(gen);
        float phase2 = offset(gen);
        Geometry::coordinate collision1 = { dist_at_sep1,0.0f };
        collision1.rotate(phase1);
        collision1.move_to(target);
        Geometry::coordinate collision2 = { dist_at_sep2, 0.0f };
        collision2.rotate(phase2);
        collision2.move_to(target);
        float R1 = rad2(gen);
        float R2 = rad2(gen);

        float phase3 = offset(gen);
        float phase4 = offset(gen);

        Geometry::coordinate pivot1 = { R1,0.0f };
        pivot1.rotate(phase3);
        pivot1.move_to(collision1);

        Geometry::coordinate pivot2 = { R2,0.0f };
        pivot2.rotate(phase4);
        pivot2.move_to(collision2);

        Motion::CCW_Rotation baka1;
        baka1.pivot_ = pivot1;
        Motion::CW_Rotation baka2;
        baka2.pivot_ = pivot2;

        Geometry::coordinate center1 = collision1 - pivot1;
        center1.rotate(-time_at_sep);
        center1.move_to(pivot1);
        Coarse_Collidable Tester1;
        Tester1.center_ = center1;
        Tester1.radius_ = r1;

        Geometry::coordinate debug1 = center1 - pivot1;

        Geometry::coordinate center2 = collision2 - pivot2;
        center2.rotate(time_at_sep);
        center2.move_to(pivot2);
        Coarse_Collidable Tester2;
        Tester2.center_ = center2;
        Tester2.radius_ = r2;

        //don't false assumptions
        if (Tester1.center_.distance_to(Tester2.center_) <= (Tester1.radius_ + Tester2.radius_)) continue;

        bool result = Tester1.collide(Tester2, baka1, baka2);
        if (!result) {
            failures += 1;
        }
    }
    //expect it to catch over 99.95 % of hard random collisions
    EXPECT_LT(failures,200);





}


TEST(Coarse_Collision_Tests, Combined_Collisions_CCW) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = -1000.0f;
    float upperBound = 1000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.1, 1000.0);
    std::uniform_real_distribution<float> rad2(0.1, 10000.0);
    std::uniform_real_distribution<float> phase(0, 1.0471975512);
    std::uniform_real_distribution<float> offset(0, 6.28318530718);
    int failures = 0;
    int trials = 0;

    for (int i = 0; i < iterations; ++i) {
        float x = dis(gen);
        float y = dis(gen);
        float r1 = rad(gen);
        float r2 = rad(gen);
        Coarse_Collidable C1;
        Geometry::coordinate target = { x,y };
        std::uniform_real_distribution<float> D2(0, 0.495f * (r1 + r2));
        float dist_at_sep1 = D2(gen);
        float dist_at_sep2 = D2(gen);
        float time_at_sep = phase(gen);
        float phase1 = offset(gen);
        float phase2 = offset(gen);
        Geometry::coordinate collision1 = { dist_at_sep1,0.0f };
        collision1.rotate(phase1);
        collision1.move_to(target);
        Geometry::coordinate collision2 = { dist_at_sep2, 0.0f };
        collision2.rotate(phase2);
        collision2.move_to(target);
        float R1 = rad2(gen);
        float R2 = rad2(gen);

        float phase3 = offset(gen);
        float phase4 = offset(gen);

        Geometry::coordinate pivot1 = { R1,0.0f };
        pivot1.rotate(phase3);
        pivot1.move_to(collision1);

        Geometry::coordinate pivot2 = { 1.0f,0.0f };
        pivot2.rotate(phase4);

        Motion::CCW_Rotation baka1;
        baka1.pivot_ = pivot1;
        Motion::Translation baka2;
        baka2.delta_ = pivot2;

        Geometry::coordinate center1 = collision1 - pivot1;
        center1.rotate(-time_at_sep);
        center1.move_to(pivot1);
        Coarse_Collidable Tester1;
        Tester1.center_ = center1;
        Tester1.radius_ = r1;

        Geometry::coordinate debug1 = center1 - pivot1;

        Geometry::coordinate center2 = pivot2 * (-0.954929658551f * time_at_sep);
        center2.move_to(collision2);
        Coarse_Collidable Tester2;
        Tester2.center_ = center2;
        Tester2.radius_ = r2;

        //don't allow false assumptions
        if (Tester1.center_.distance_to(Tester2.center_) <= (Tester1.radius_ + Tester2.radius_)) continue;

        bool result = Tester1.collide(Tester2, baka1, baka2);
        if (!result) {
            failures += 1;
        }
    }
    EXPECT_LT(failures, 100);


}



TEST(Coarse_Collision_Tests, Combined_Collisions_CW) {
    std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = -1000.0f;
    float upperBound = 1000.0f;

    std::uniform_real_distribution<float> dis(lowerBound, upperBound);
    std::uniform_real_distribution<float> rad(0.1, 1000.0);
    std::uniform_real_distribution<float> rad2(0.1, 10000.0);
    std::uniform_real_distribution<float> phase(0, 1.0471975512);
    std::uniform_real_distribution<float> offset(0, 6.28318530718);
    int failures = 0;
    int trials = 0;

    for (int i = 0; i < iterations; ++i) {
        float x = dis(gen);
        float y = dis(gen);
        float r1 = rad(gen);
        float r2 = rad(gen);
        Coarse_Collidable C1;
        Geometry::coordinate target = { x,y };
        std::uniform_real_distribution<float> D2(0, 0.495f * (r1 + r2));
        float dist_at_sep1 = D2(gen);
        float dist_at_sep2 = D2(gen);
        float time_at_sep = phase(gen);
        float phase1 = offset(gen);
        float phase2 = offset(gen);
        Geometry::coordinate collision1 = { dist_at_sep1,0.0f };
        collision1.rotate(phase1);
        collision1.move_to(target);
        Geometry::coordinate collision2 = { dist_at_sep2, 0.0f };
        collision2.rotate(phase2);
        collision2.move_to(target);
        float R1 = rad2(gen);
        float R2 = rad2(gen);

        float phase3 = offset(gen);
        float phase4 = offset(gen);

        Geometry::coordinate pivot1 = { R1,0.0f };
        pivot1.rotate(phase3);
        pivot1.move_to(collision1);

        Geometry::coordinate pivot2 = { 1.0f,0.0f };
        pivot2.rotate(phase4);

        Motion::CW_Rotation baka1;
        baka1.pivot_ = pivot1;
        Motion::Translation baka2;
        baka2.delta_ = pivot2;

        Geometry::coordinate center1 = collision1 - pivot1;
        center1.rotate(time_at_sep);
        center1.move_to(pivot1);
        Coarse_Collidable Tester1;
        Tester1.center_ = center1;
        Tester1.radius_ = r1;

        Geometry::coordinate debug1 = center1 - pivot1;

        Geometry::coordinate center2 = pivot2 * (-0.954929658551f * time_at_sep);
        center2.move_to(collision2);
        Coarse_Collidable Tester2;
        Tester2.center_ = center2;
        Tester2.radius_ = r2;

        //don't allow false assumptions
        if (Tester1.center_.distance_to(Tester2.center_) <= (Tester1.radius_ + Tester2.radius_)) continue;

        bool result = Tester1.collide(Tester2, baka1, baka2);
        if (!result) {
            failures += 1;
        }
    }
    EXPECT_LT(failures, 100);

    


}