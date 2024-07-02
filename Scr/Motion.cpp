#include "Motion.hpp"


Motion::Motion_Type::ids_ Motion::Motion_Type::getId() const {
	return id;	
}

Motion::Motion_Type::Motion_Type(const Motion_Type::ids_& val) : id(val) {}

Motion::CCW_Rotation::CCW_Rotation() : pivot_({ 0.0f,0.0f }) , Motion_Type(ids_::ccw_rot) {}

Motion::CW_Rotation::CW_Rotation() : pivot_({ 0.0f,0.0f }) , Motion_Type(ids_::cw_rot) {}

Motion::Translation::Translation() : delta_({ 0.0f,0.0f }) , Motion_Type(ids_::trans) {}

Motion::Stationary::Stationary() : data_({ 0.0f,0.0f }) , Motion_Type(ids_::stationary) {}

Motion::CCW_Rotation::CCW_Rotation(const Geometry::coordinate &coord) : pivot_(coord), Motion_Type(ids_::ccw_rot) {}

Motion::CW_Rotation::CW_Rotation(const Geometry::coordinate& coord) : pivot_(coord), Motion_Type(ids_::cw_rot) {}

Motion::Translation::Translation(const Geometry::coordinate& coord) : delta_(coord), Motion_Type(ids_::trans) {}


bool Motion::CCW_Rotation::operator==(const CCW_Rotation& other) const {
	return other.pivot_ == pivot_ && other.id == other.id;
}

bool Motion::Translation::operator==(const Translation& other) const {
	return other.delta_ == delta_ && other.id == other.id;
}

bool Motion::CW_Rotation::operator==(const CW_Rotation& other) const {
	return other.pivot_ == pivot_ && other.id == other.id;
}






Collider::rectangle Motion::Stationary::Generate_Bounding_Box(const Geometry::Circle& obj) const {
	Collider::rectangle output;
	output.LX = int(obj.center_.x - obj.radius_) - 1;
	output.UX = int(obj.center_.x + obj.radius_) + 1;
	output.LY = int(obj.center_.y - obj.radius_) - 1;
	output.UY = int(obj.center_.y + obj.radius_) + 1;
	return output;

}

Collider::rectangle Motion::Translation::Generate_Bounding_Box(const Geometry::Circle& obj) const {
	Collider::rectangle output;
	if (delta_.x > 0) {
		output.LX = int(obj.center_.x - obj.radius_);
		output.UX = int(obj.center_.x + obj.radius_ + delta_.x) + 1;
	}
	else {
		output.LX = int(obj.center_.x - obj.radius_ + delta_.x);
		output.UX = int(obj.center_.x + obj.radius_) + 1;
	}
	if (delta_.y > 0) {
		output.LY = int(obj.center_.y - obj.radius_);
		output.UY = int(obj.center_.y + obj.radius_ + delta_.y) + 1;
	}
	else {
		output.LY = int(obj.center_.y - obj.radius_ + delta_.y);
		output.UY = int(obj.center_.y + obj.radius_) + 1;
	}
	return output;
}


Collider::rectangle Motion::CCW_Rotation::Generate_Bounding_Box(const Geometry::Circle& obj) const {
	Collider::rectangle output;
	Geometry::coordinate phase = obj.center_ - pivot_;
	float radius = sqrt((phase.x * phase.x) + (phase.y * phase.y));
	float prev_x = phase.x;
	float prev_y = phase.y;
	bool sign_x = phase.x > 0;
	bool sign_y = phase.y > 0;
	phase.rotate(1.0471975512);
	bool new_sign_x = phase.x > 0;
	bool new_sign_y = phase.y > 0;
	float LX, UX, UY, LY;
	if (sign_y ^ new_sign_y) {
		if (sign_y) {
			LX = -radius;
			UX = std::max(prev_x,phase.x);
		}
		else {
			UX = radius;
			LX = std::min(prev_x, phase.x);
		}
	}
	else {
		UX = std::max(prev_x, phase.x);
		LX = std::min(prev_x, phase.x);
	}

	if (sign_x ^ new_sign_x) {
		if (sign_x) {
			UY = radius;
			LY = std::min(prev_y, phase.y);
		}
		else {
			LY = -radius;
			UY = std::max(prev_y, phase.y);
		}
	}
	else {
		UY = std::max(prev_y, phase.y);
		LY = std::min(prev_y, phase.y);
	}
	output.LX = int(LX + pivot_.x - obj.radius_) - 1;
	output.LY = int(LY + pivot_.y - obj.radius_) - 1;
	output.UY = int(UY + pivot_.y + obj.radius_) + 1;
	output.UX = int(UX + pivot_.x + obj.radius_) + 1;

	return output;
}




Collider::rectangle Motion::CW_Rotation::Generate_Bounding_Box(const Geometry::Circle& obj) const {
	Collider::rectangle output;
	Geometry::coordinate phase = obj.center_ - pivot_;
	float radius = sqrt((phase.x * phase.x) + (phase.y * phase.y));
	float prev_x = phase.x;
	float prev_y = phase.y;
	bool new_sign_x = phase.x > 0;
	bool new_sign_y = phase.y > 0;
	phase.rotate(-1.0471975512);
	bool sign_x = phase.x > 0;
	bool sign_y = phase.y > 0;
	float LX, UX, UY, LY;
	if (sign_y ^ new_sign_y) {
		if (sign_y) {
			LX = -radius;
			UX = std::max(prev_x, phase.x);
		}
		else {
			UX = radius;
			LX = std::min(prev_x, phase.x);
		}
	}
	else {
		UX = std::max(prev_x, phase.x);
		LX = std::min(prev_x, phase.x);
	}

	if (sign_x ^ new_sign_x) {
		if (sign_x) {
			UY = radius;
			LY = std::min(prev_y, phase.y);
		}
		else {
			LY = -radius;
			UY = std::max(prev_y, phase.y);
		}
	}
	else {
		UY = std::max(prev_y, phase.y);
		LY = std::min(prev_y, phase.y);
	}
	output.LX = int(LX + pivot_.x - obj.radius_) - 1;
	output.LY = int(LY + pivot_.y - obj.radius_) - 1;
	output.UY = int(UY + pivot_.y + obj.radius_) + 1;
	output.UX = int(UX + pivot_.x + obj.radius_) + 1;

	return output;
}



Geometry::Rigid_Transform Motion::CCW_Rotation::getTransform(const double& InterpolationFactor) const {
	Geometry::Rigid_Transform output;
	output.Translation -= pivot_;
	output.Translation.rotate(1.0471975512 * InterpolationFactor);
	output.Translation += pivot_;
	output.Rotation = { (float)cos(1.0471975512 * InterpolationFactor), (float)sin(1.0471975512 * InterpolationFactor) };
	return output;
}

Geometry::Rigid_Transform Motion::CW_Rotation::getTransform(const double& InterpolationFactor) const {
	Geometry::Rigid_Transform output;
	output.Translation -= pivot_;
	output.Translation.rotate(-1.0471975512 * InterpolationFactor);
	output.Translation += pivot_;
	output.Rotation = { (float)cos(1.0471975512 * InterpolationFactor), (float)sin(-1.0471975512 * InterpolationFactor) };
	return output;
}

Geometry::Rigid_Transform Motion::Stationary::getTransform(const double& InterpolationFactor) const {
	Geometry::Rigid_Transform output;
	return output;
}

Geometry::Rigid_Transform Motion::Translation::getTransform(const double& InterpolationFactor) const {
	Geometry::Rigid_Transform output;
	output.Translation = delta_ * InterpolationFactor;
	return output;
}