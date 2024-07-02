#include "Collidable.hpp"
#include "Utils.hpp"

bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::Stationary&, const Motion::Stationary&) const {
	return center_.distance_to(target.center_) < radius_ + target.radius_;
}



bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::CCW_Rotation& M2) const {

	float R1 = center_.distance_to(M1.pivot_);
	float R2 = target.center_.distance_to(M2.pivot_);

	//unitless distance between 2 pivots
	float X = (M1.pivot_.x - M2.pivot_.x) / R1;
	float Y = (M1.pivot_.y - M2.pivot_.y) / R1;

	//minimum distance^2 before collision with a little bit of slop for floating point errors
	float Q = (radius_ + target.radius_) * (radius_ + target.radius_) * 1.0000001f;

	//ratio of gobbledegook
	float R = R2 / R1;

	//pointing vectors
	Geometry::coordinate p1 = center_ - M1.pivot_;
	Geometry::coordinate p2 = target.center_ - M2.pivot_;

	//normalized pointing vector to objects from rotation center		
	Geometry::coordinate t1 = (center_ - M1.pivot_) / R1;
	Geometry::coordinate t2 = (target.center_ - M2.pivot_) / R2;

	//inverse change of variables
	float phase_factor = atan2f(t2.y, t2.x);

	//maximum duration = pi / 3
	float cos_phase_dif = t2.x * t1.x + t1.y * t2.y;
	float sin_phase_dif = t1.y * t2.x - t1.x * t2.y;
	float numerator = Y * sin_phase_dif + X * cos_phase_dif - R * X;
	float denominator = std::sqrt((X * X + Y * Y) * (R * R - 2 * R * cos_phase_dif + 1));
	float candidate_1 = acos(numerator / denominator);
	float candidate_2 = Geometry::constrain_angles(3.14159265359f - candidate_1 - phase_factor);
	float candidate_3 = Geometry::constrain_angles(-candidate_1 - phase_factor);
	float candidate_4 = Geometry::constrain_angles(candidate_1 - 3.14159265359f - phase_factor);
	candidate_1 = Geometry::constrain_angles(candidate_1 - phase_factor);

	float candidate_5 = 1.0471975512f;
	bool return_value_1 = false, return_value_2 = false, return_value_3 = false, return_value_4 = false, return_value_5 = false;

	auto distance = [&](const float& angle) {
		Geometry::coordinate P1 = p1;
		Geometry::coordinate P2 = p2;
		P1.rotate(angle);
		P2.rotate(angle);
		P1.move_to(M1.pivot_);
		P2.move_to(M2.pivot_);
		return ((P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y) < Q);
	};

	return_value_1 = candidate_1 < candidate_5 && distance(candidate_1);
	return_value_2 = candidate_2 < candidate_5 && distance(candidate_2);
	return_value_3 = candidate_3 < candidate_5 && distance(candidate_3);
	return_value_4 = candidate_4 < candidate_5 && distance(candidate_4);
	return_value_5 = distance(candidate_5);
	return return_value_1 || return_value_2 || return_value_3 || return_value_4 || return_value_5;
}


bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::CW_Rotation& M2) const {
	float R1 = center_.distance_to(M1.pivot_);
	float R2 = target.center_.distance_to(M2.pivot_);
	float Q_0 = (target.radius_ + radius_) * (target.radius_ + radius_);
	//unitless distance between 2 pivots
	float X_0 = (M1.pivot_.x - M2.pivot_.x);
	float Y_0 = (M1.pivot_.y - M2.pivot_.y);
	float X = X_0 / R2;
	float Y = Y_0 / R2;
	//minimum distance^2 before collision with a little bit of slop for floating point errors
	float Q = Q_0 / (R2 * R2);

	//ratio of gobbledegook
	float R = R1 / R2;

	//pointing vectors
	Geometry::coordinate p1 = center_ - M1.pivot_;
	Geometry::coordinate p2 = target.center_ - M2.pivot_;

	Geometry::coordinate t1 = p1 / R1;
	Geometry::coordinate t2 = p2 / R2;

	float cos_t1 = t1.x;
	float sin_t1 = t1.y;
	float cos_t2 = t2.x;
	float sin_t2 = t2.y;


	float sin_t1_t2 = sin_t1 * cos_t2 - sin_t2 * cos_t1;
	float cos_t1_t2 = cos_t1 * cos_t2 + sin_t1 * sin_t2;

	//cubic coefficients for interpolation of the derivative
	float a, b, c, d;
	
	//magic constants for hermite interpolation
	a = X * (-0.16439689223f * sin_t1 + 0.28474377013f * cos_t1) + Y * (0.2847437701f * sin_t1 + 0.16439689f * cos_t1);
	a += (4.372060752f * sin_t1_t2 - 2.5242104522f * cos_t1_t2);
	a = a * R + X * (-0.16439689233f * sin_t2 - 0.28474377013691f * cos_t2) + Y * (-0.2847437701369f * sin_t2 + 0.16439689233f * cos_t2);

	b = R * (X * (1.1046538841914f * sin_t1 + 0.0414062597875464f * cos_t1) + Y * (0.0414062597875464f * sin_t1 - 1.1046538841914f * cos_t1) - 10.5151795813757f * sin_t1_t2 - 2.35274325978964f * cos_t1_t2);
	b += X * (1.1046538841914f * sin_t2 - 0.0414062597875464f * cos_t2) + Y * (-0.0414062597875464f * sin_t2 - 1.1046538841914f * cos_t2);

	c = (X*(-0.02157981809f * sin_t1 - 2.00960370533f * cos_t1) + Y * (-2.009603705333f * sin_t1 + 0.02157981809915f * cos_t1)) ;
	c += 0.487391240831f * sin_t1_t2 + 8.53986686265376f * cos_t1_t2;
	c = R * c + X * (-0.0215798180991555f * sin_t2 + 2.00960370533311f * cos_t2) + Y * (2.00960370533311f * sin_t2 + 0.0215798180991f * cos_t2);
	d = -2.0 * R * X * sin_t1 + 2.0 * R * Y * cos_t1 + 4.0 * R * sin_t1_t2 - 2.0 * X * sin_t2 + 2.0 * Y * cos_t2;
	a = 1.0f / a;
	d *= a;
	c *= a;
	b *= a;
	float candidates[3] = {-1,-1,-1};
	Arrithmetic::cubic_solver(b, c, d, candidates);
	bool results;
	auto check_distance = [&](const float& value) {
		float X_component = X + R * (cos_t1 * cos(value) - sin_t1 * sin(value)) + (cos_t2*cos(value) + sin_t2 * sin(value));
		float Y_component = Y + R * (cos_t1 * sin(value) + sin_t1 * cos(value)) + (sin_t2 * cos(value) - cos_t2 * sin(value));
		return X_component * X_component + Y_component * Y_component < Q;
	};
	for (int i = 0; i < 3; ++i) {
		if (candidates[i] > 0 && candidates[i] < 1.0471975512) {
			//one final newton raphson to correct for the intepolation
			float value = candidates[i];
			float numerator = Y * (cos_t2 * cos(value) + sin_t2 * sin(value)) + R * Y * (cos_t1 * cos(value) - sin_t1 * sin(value));
			numerator -= X * (cos(value) * sin_t2 - sin(value) * cos_t2) + X * R * (cos_t1 * sin(value) + sin_t1 * cos(value));
			numerator += 2 * R * (cos_t1_t2 * sin(2.0f * value) + sin_t1_t2 * cos(2.0f * value));

			float denom = R * X * (cos_t1 * cos(value) - sin_t1 * sin(value)) + R * Y * (cos_t1 * sin(value) + sin_t1 * cos(value));
			denom -= X * (cos_t2 * cos(value) + sin_t2 * sin(value)) + Y * (cos(value) * sin_t2 - sin(value) * cos_t2);
			denom -= 4 * R * (cos_t1_t2 * cos(2.0f * value) - sin(2.0f * value) * sin_t1_t2);
			candidates[i] += numerator / denom;


			float X_component = X + R * (cos_t1 * cos(candidates[i]) - sin_t1 * sin(candidates[i])) - (cos_t2 * cos(candidates[i]) + sin_t2 * sin(candidates[i]));
			float Y_component = Y + R * (cos_t1 * sin(candidates[i]) + sin_t1 * cos(candidates[i])) - (sin_t2 * cos(candidates[i]) - cos_t2 * sin(candidates[i]));
			if (X_component * X_component + Y_component * Y_component < Q) {
				return true;
			}
		}
	}
	float X_component = X + R * (cos_t1 * cos(1.0471975512) - sin_t1 * sin(1.0471975512)) - (cos_t2 * cos(1.0471975512) + sin_t2 * sin(1.0471975512));
	float Y_component = Y + R * (cos_t1 * sin(1.0471975512) + sin_t1 * cos(1.0471975512)) - (sin_t2 * cos(1.0471975512) - cos_t2 * sin(1.0471975512));
	if (X_component * X_component + Y_component * Y_component < Q) {
		return true;
	}
	return false;



}

bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::CW_Rotation& M1, const Motion::CW_Rotation& M2) const {
	float R1 = center_.distance_to(M1.pivot_);
	float R2 = target.center_.distance_to(M2.pivot_);

	//unitless distance between 2 pivots
	float X = (M1.pivot_.x - M2.pivot_.x) / R1;
	float Y = (M1.pivot_.y - M2.pivot_.y) / R1;

	//minimum distance^2 before collision with a little bit of slop for floating point errors
	float Q = (radius_ + target.radius_) * (radius_ + target.radius_) * 1.0000001f;

	//ratio of gobbledegook
	float R = R2 / R1;

	//pointing vectors
	Geometry::coordinate p1 = center_ - M1.pivot_;
	Geometry::coordinate p2 = target.center_ - M2.pivot_;

	//normalized pointing vector to objects from rotation center		
	Geometry::coordinate t1 = (center_ - M1.pivot_) / R1;
	Geometry::coordinate t2 = (target.center_ - M2.pivot_) / R2;

	//inverse change of variables
	float phase_factor = atan2f(t2.y, t2.x);

	//maximum duration = pi / 3
	float cos_phase_dif = t2.x * t1.x + t1.y * t2.y;
	float sin_phase_dif = t1.y * t2.x - t1.x * t2.y;
	float numerator = Y * sin_phase_dif + X * cos_phase_dif - R * X;
	float denominator = std::sqrt((X * X + Y * Y) * (R * R - 2 * R * cos_phase_dif + 1));
	float candidate_1 = acos(numerator / denominator);
	float candidate_2 = Geometry::constrain_angles(3.14159265359f - candidate_1 - phase_factor);
	float candidate_3 = Geometry::constrain_angles(-candidate_1 - phase_factor);
	float candidate_4 = Geometry::constrain_angles(candidate_1 - 3.14159265359f - phase_factor);
	candidate_1 = Geometry::constrain_angles(candidate_1 - phase_factor);

	float candidate_5 = 4.18879020479f;
	bool return_value_1 = false, return_value_2 = false, return_value_3 = false, return_value_4 = false, return_value_5 = false;

	auto distance = [&](const float& angle) {
		Geometry::coordinate P1 = p1;
		Geometry::coordinate P2 = p2;
		P1.rotate(angle);
		P2.rotate(angle);
		P1.move_to(M1.pivot_);
		P2.move_to(M2.pivot_);
		return ((P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y) < Q);
	};

	return_value_1 = candidate_1 > candidate_5&& distance(candidate_1);
	return_value_2 = candidate_2 > candidate_5&& distance(candidate_2);
	return_value_3 = candidate_3 > candidate_5&& distance(candidate_3);
	return_value_4 = candidate_4 > candidate_5&& distance(candidate_4);
	return_value_5 = distance(candidate_5);
	return return_value_1 || return_value_2 || return_value_3 || return_value_4 || return_value_5;
}

bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::CCW_Rotation& M1, const Motion::Translation& M2) const {
	float R = 1.0471975512f * center_.distance_to(M1.pivot_);

	//pointing vector
	Geometry::coordinate p1 = center_ - M1.pivot_;

	float Q = 1.09662271123f * ((target.radius_ + radius_) * (target.radius_ + radius_));

	Geometry::coordinate D = M1.pivot_ - target.center_;
	D.counter_rotate(M2.delta_.x, M2.delta_.y);
	p1.counter_rotate(M2.delta_.x, M2.delta_.y);
	float X = D.x * 1.0471975512f;
	float Y = D.y * 1.0471975512f;
	float t = atan2f(p1.y, p1.x);
	float phase = atan2f(-D.y, -D.x);
	float guess1 = Geometry::constrain_angles(phase - t);
	phase = atan2f(-D.y, 1.0f - D.x);
	float guess2 = Geometry::constrain_angles(phase - t);
	float A = R * (Y - 1.0f);
	auto Newton_Raphson = [&](float &value) {
		float numerator = A * cos(t + value) + (value - X) * (1.0f + R * sin(t + value));
		float denominator = (R - A) * sin(t + value) + R *(value - X) * cos(t + value) + 1.0f;
		float temp = value - numerator / denominator;
		if (temp < 0) {
			value *= 0.5f;
		}
		else if (temp > 1.0471975512f) {
			value = 0.5f * (value + 1.0471975512f);
		}
		else{
			value -= numerator / denominator;
		}
	};
	auto distance = [&](const float& value) {
		float temp_x = (X + R * cos(value + t) - value);
		float temp_y = (Y + R * sin(value + t));
		return (temp_x * temp_x) + (temp_y * temp_y) < Q;

	};
	if (guess1 < 1.0471975512f && guess1 > 0) {
		Newton_Raphson(guess1);
		if (distance(guess1)) {
			return true;
		}
	}

	if (guess2 < 1.0471975512f && guess2 > 0) {
		Newton_Raphson(guess2);
		if (distance(guess2)) {
			return true;
		}
	}

	return distance(1.0471975512f);
}

bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::Translation& M1, const Motion::CCW_Rotation& M2) const {
	return target.collide(*this, M2, M1);
}

bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::CW_Rotation &M1, const Motion::Translation& M2) const {
	float R = 1.0471975512f * center_.distance_to(M1.pivot_);

	//pointing vector
	Geometry::coordinate p1 = { M1.pivot_.x - center_.x ,center_.y - M1.pivot_.y };

	float Q = 1.09662271123f * ((target.radius_ + radius_) * (target.radius_ + radius_));
	Geometry::coordinate D = { target.center_.x - M1.pivot_.x, M1.pivot_.y - target.center_.y };

	D.counter_rotate(-M2.delta_.x, M2.delta_.y);
	p1.counter_rotate(-M2.delta_.x, M2.delta_.y);
	float X = D.x * 1.0471975512f;
	float Y = D.y * 1.0471975512f;
	float t = atan2f(p1.y, p1.x);
	float phase = atan2f(-D.y, -D.x);
	float guess1 = Geometry::constrain_angles(phase - t);
	phase = atan2f(-D.y, 1.0f - D.x);
	float guess2 = Geometry::constrain_angles(phase - t);
	float A = R * (Y - 1.0f);
	auto Newton_Raphson = [&](float& value) {
		float numerator = A * cos(t + value) + (value - X) * (1.0f + R * sin(t + value));
		float denominator = (R - A) * sin(t + value) + R * (value - X) * cos(t + value) + 1.0f;
		float temp = value - numerator / denominator;
		if (temp < 0) {
			value *= 0.5f;
		}
		else if (temp > 1.0471975512f) {
			value = 0.5f * (value + 1.0471975512f);
		}
		else {
			value -= numerator / denominator;
		}
	};
	auto distance = [&](const float& value) {
		float temp_x = (X + R * cos(value + t) - value);
		float temp_y = (Y + R * sin(value + t));
		return (temp_x * temp_x) + (temp_y * temp_y) < Q;

	};
	if (guess1 < 1.0471975512f && guess1 > 0) {
		Newton_Raphson(guess1);
		if (distance(guess1)) {
			return true;
		}
	}

	if (guess2 < 1.0471975512f && guess2 > 0) {
		Newton_Raphson(guess2);
		if (distance(guess2)) {
			return true;
		}
	}

	return distance(1.0471975512f);

}



bool Coarse_Collidable::collide(const Coarse_Collidable& target, const Motion::Translation &M1, const Motion::CW_Rotation& M2) const {

	return target.collide(*this, M2, M1);

}