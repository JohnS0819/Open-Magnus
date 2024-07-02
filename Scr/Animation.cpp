#include "Animation.hpp"
#include "Time.hpp"

Motion_Animation<Motion::CW_Rotation>::Motion_Animation(const Motion::CW_Rotation& variable) : motion(variable) {}
Motion_Animation<Motion::CCW_Rotation>::Motion_Animation(const Motion::CCW_Rotation& variable) : motion(variable) {}
Motion_Animation<Motion::Translation>::Motion_Animation(const Motion::Translation& variable) : motion(variable) {}

sf::Transform Motion_Animation<Motion::CCW_Rotation>::getTransform() {
	float angle = 60.0f * Time_Singleton::instance().getInterpolation();

	sf::Transform output;
	output.rotate(angle, { Geometry::SPACE_TO_PIXELS * motion.pivot_.x,Geometry::SPACE_TO_PIXELS * motion.pivot_.y });
	return output;
}

sf::Transform Motion_Animation<Motion::CW_Rotation>::getTransform() {
	float angle = -60.0f * Time_Singleton::instance().getInterpolation();
	sf::Transform output;
	output.rotate(angle, { Geometry::SPACE_TO_PIXELS * motion.pivot_.x,Geometry::SPACE_TO_PIXELS * motion.pivot_.y });
	return output;
}

sf::Transform Motion_Animation<Motion::Translation>::getTransform() {
	sf::Transform output;
	float amount = Time_Singleton::instance().getInterpolation();
	output.translate({ Geometry::SPACE_TO_PIXELS * amount * motion.delta_.x, Geometry::SPACE_TO_PIXELS * amount * motion.delta_.y });
	return output;
}
