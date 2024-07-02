#include "pch.h"
#include <random>
#include "Utils.hpp"

TEST(ARRITHMETIC_TESTS, cubic_solver) {

    using namespace Arrithmetic;
    std::random_device rd;
    std::mt19937 gen(rd());
    // Define the range
    float lowerBound = 0.0f;
    float upperBound = 4.0f;
    int errors = 0;

    std::uniform_real_distribution<float> dis2(1.15, upperBound);
    std::uniform_real_distribution<float> dis3(-3.0, -0.05);
    std::uniform_real_distribution<float> dis4(-10.0, 10.0);
    std::uniform_real_distribution<float> dis(0.0, 1.047);
    double error = 0.0;
    for (int i = 0; i < 10000; ++i) {
        std::vector<float> roots(3, 0.0);
        roots[0] = dis(gen);
        roots[1] = dis(gen);
        roots[2] = dis2(gen);
        std::sort(roots.begin(), roots.end());
        float d = -roots[0] * roots[1] * roots[2];
        float b = -roots[0] - roots[1] - roots[2];
        float c = (roots[0] * roots[1]) + (roots[1] * roots[2]) + (roots[0] * roots[2]);
        float initial[3];
        cubic_solver(b, c, d, initial);
        for (int j = 0; j < 2; ++j) {
            float baba = 100000.0f;
            for (int k = 0; k < 3; ++k) {
                baba = fmin(baba, fabs(roots[j] - initial[k]));
            }
            if (baba >= 0.02) {
                ++errors;
            }
            //EXPECT_LT(baba, 0.02) << std::endl << roots[0] << '\t' << roots[1] << '\t' << roots[2] << std::endl << initial[0] << '\t' << initial[1] << '\t' << initial[2];
            error += baba;
        }
    }
    for (int i = 0; i < 10000; ++i) {
        std::vector<float> roots(3, 0.0);
        roots[0] = dis(gen);
        roots[1] = dis(gen);
        roots[2] = dis3(gen);
        std::sort(roots.begin(), roots.end());

        float d = -roots[0] * roots[1] * roots[2];
        float b = -roots[0] - roots[1] - roots[2];
        float c = (roots[0] * roots[1]) + (roots[1] * roots[2]) + (roots[0] * roots[2]);
        float initial[3];


        cubic_solver(b, c, d, initial);

        for (int j = 1; j < 3; ++j) {
            float baba = 100000.0f;
            for (int k = 0; k < 3; ++k) {
                baba = fmin(baba, fabs(roots[j] - initial[k]));
            }
            if (baba >= 0.02) {
                ++errors;
            }
            //EXPECT_LT(baba, 0.02) << std::endl << roots[0] << '\t' << roots[1] << '\t' << roots[2] << std::endl << initial[0] << '\t' << initial[1] << '\t' << initial[2];
            error += baba;
        }
    }
    for (int i = 0; i < 10000; ++i) {
        std::vector<float> roots(3, 0.0);
        roots[0] = dis(gen);
        roots[1] = dis4(gen);
        roots[2] = dis4(gen);
        std::sort(roots.begin(), roots.end());

        float d = -roots[0] * roots[1] * roots[2];
        float b = -roots[0] - roots[1] - roots[2];
        float c = (roots[0] * roots[1]) + (roots[1] * roots[2]) + (roots[0] * roots[2]);
        float initial[3];


        cubic_solver(b, c, d, initial);

        for (int j = 0; j < 3; ++j) {
            float baba = 100000.0f;
            if (roots[j] < 0.0 || roots[j] > 1.0471975512) {
                continue;
            }
            for (int k = 0; k < 3; ++k) {
                baba = fmin(baba, fabs(roots[j] - initial[k]));
            }
            if (baba >= 0.02) {
                ++errors;
            }
            //EXPECT_LT(baba, 0.02) << std::endl << roots[0] << '\t' << roots[1] << '\t' << roots[2] << std::endl << initial[0] << '\t' << initial[1] << '\t' << initial[2];
            error += baba;
        }
    }
    EXPECT_LT(error, 100.0);
    EXPECT_LT(errors, 3);

}
