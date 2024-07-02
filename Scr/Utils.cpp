#include "Utils.hpp"

std::pair<float, float> Arrithmetic::quadratic_solver(const float& a, const float& b, const float& c) {
    float dis = b * b - 4.0f * a * c;
    if (dis < 0.0f) {
        return { -1.0f,-1.0f };
    }
    float denom = (b + ((b < 0) ? -1.0f : 1.0f) * sqrt(dis));
    float lower = -2.0f * c / denom;
    float upper = denom / (-2.0f * a);
    if (lower > upper) {
        std::swap(lower, upper);
    }
    return { lower, upper };

}


void Arrithmetic::cubic_solver(const float& b, const float& c, const float& d, float* output){
    float a_ = 3.0f;
    float b_ = 2.0f * b;
    float c_ = c;
    auto interval = quadratic_solver(a_, b_, c_);
    bool sign_at_0 = d < 0.0f;
    bool sign_at_pi_3 = (d + c * 1.0471975512f + b * 1.09662271123f + 1.14838061779f) < 0.0f;
    //hybrid newton-raphson / bisection approach, Guarentees at least linear convergence
    auto Newton_Raphson = [&](float& current, float& lower, float& upper, const bool& sign_at_the_lower) {
        float square = current * current;
        float cube = current * square;
        float value = cube + square * b + current * c + d;
        bool sign_of_value = value < 0.0f;
        if (sign_of_value ^ sign_at_the_lower) {
            upper = current;
        }
        else {
            lower = current;
        }
        float derivative = a_ * square + b_ * current + c_;
        if (derivative < 0.0000001f && derivative > -0.0000001f) {
            return;
        }
        float ratio = current - value / derivative;
        if (ratio < lower || ratio > upper) {
            current = 0.5f * (lower + upper);
        }
        else {
            current = ratio;
        }
        return;
    };
    auto Modified_Newton_Raphson = [&](float& current) {
        float square = current * current;
        float cube = current * square;
        float value = cube + square * b + current * c + d;
        float derivative = a_ * square + b_ * current + c_;
        float derivative2 = 6.0f * current + b_;
        float denom = (derivative * derivative - value * derivative2);
        if (denom < 0.0000001f && denom > -0.000001f) {
            return;
        }
        float ratio = (value * derivative) / denom;
        current -= ratio;
    };

    //equation is monotonic in interval of interest (at most 1 root)
    if (interval.second < 0.0f || interval.first >= 1.0471975512f || (interval.first <= 0.0f && interval.second >= 1.0471975512f)) {
        if (sign_at_0 ^ sign_at_pi_3) {
            //we can precompute these values
            float initial = 0.143547577224 + b * 0.274155677808 + c * 0.523598775598 + d;
            float lower_ = 0.0f;
            float upper_ = 1.14838061779f;
            bool sign_initial = initial < 0.0f;
            if (sign_initial ^ sign_at_0) {
                upper_ = 0.523598775598f;
            }
            else {
                lower_ = 0.523598775598f;
            }
            output[0] = 0.523598775598 - initial / (a_ * 0.274155677808 + b_ * 0.523598775598 + c_);
            if (output[0] < lower_ || output[0] > upper_) {
                output[0] = 0.5f * (lower_ + upper_);
            }

            for (int i = 0; i < 3; ++i) {
                Newton_Raphson(output[0], lower_, upper_, sign_at_0);
            }
            Modified_Newton_Raphson(output[0]);
        }
        return;
    }
    //at most 3 roots
    else if (interval.second < 1.0471975512f && interval.first > 0.0f) {
        output[0] = interval.first;
        output[1] = interval.second;
        output[2] = interval.second;
        float First = interval.first;
        float value1 = First * First * First + b * First * First + c * First + d;
        float Second = interval.second;
        float value2 = Second * Second * Second + b * Second * Second + c * Second + d;
        bool sign_at_first = value1 < 0.0f;
        bool sign_at_second = value2 < 0.0f;
        if (sign_at_0 ^ sign_at_first) {
            if ((-3.0 * First > b) ^ sign_at_first) {
                output[0] = First - sqrt(value1 / (-3.0 * First - b));

            }
            else {
                output[0] = 0.5f * First;
            }
            float lower = 0.0f;
            float upper = First;
            for (int i = 0; i < 3; ++i) {
                Newton_Raphson(output[0], lower, upper, sign_at_0);
            }
            Modified_Newton_Raphson(output[0]);
        }
        if (sign_at_first ^ sign_at_second) {
            float lower = First;
            float upper = Second;
            output[1] = 0.5f * (First + Second);
            for (int i = 0; i < 3; ++i) {
                Newton_Raphson(output[1], lower, upper, sign_at_first);
            }
            Modified_Newton_Raphson(output[1]);

        }
        if (sign_at_pi_3 ^ sign_at_second) {
            float lower = Second;
            float upper = 1.0471975512f;
            if ((-3.0 * Second > b) ^ (sign_at_second)) {
                output[2] = Second + sqrt(value2 / (-3.0 * Second - b));
            }
            else {
                output[2] = 0.5f * (lower + upper);
            }
            for (int i = 0; i < 3; ++i) {
                Newton_Raphson(output[2], lower, upper, sign_at_second);
            }
            Modified_Newton_Raphson(output[2]);

        }

    }
    //at most 2 roots
    else {
        float middle;
        if (interval.first > 0.0f) {
            middle = interval.first;
        }
        else {
            middle = interval.second;
        }
        float value = middle * middle * middle + b * middle * middle + c * middle + d;
        if (value > -0.0001f && value < 0.0001f) {
            output[2] = middle;
        }
        else {
            output[2] = -1;
        }
        bool sign_at_middle = value < 0.0f;
        if (sign_at_middle ^ sign_at_0) {
            float lower = 0.0f;
            float upper = middle;
            output[0] = middle - sqrt(value / (-3.0 * middle - b));
            Newton_Raphson(output[0], lower, upper, sign_at_0);
            Newton_Raphson(output[0], lower, upper, sign_at_0);
            Newton_Raphson(output[0], lower, upper, sign_at_0);
            Modified_Newton_Raphson(output[0]);

        }
        if (sign_at_middle ^ sign_at_pi_3) {
            float lower = middle;
            float upper = 1.0471975512;
            output[1] = middle + sqrt(value / (-3.0 * middle - b));
            Newton_Raphson(output[1], lower, upper, sign_at_middle);
            Newton_Raphson(output[1], lower, upper, sign_at_middle);
            Newton_Raphson(output[1], lower, upper, sign_at_middle);
            Modified_Newton_Raphson(output[1]);
        }

    }
}