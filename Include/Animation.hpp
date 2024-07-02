#ifndef ANIMATION_HPP
#define ANIMATION_HPP
#include <SFML/Graphics.hpp>
#include <chrono>
#include "Motion.hpp"
class Animation {

public:
	virtual sf::Transform getTransform() = 0;
};


template<typename T>
class Motion_Animation : public Animation{
public:
	const T& motion;

	virtual sf::Transform getTransform() = 0;

};


template<>
class Motion_Animation<Motion::CW_Rotation> : public Animation {
public:
	Motion::CW_Rotation motion;

	virtual sf::Transform getTransform();
	
	Motion_Animation(const Motion::CW_Rotation& M);

};
template<>
class Motion_Animation<Motion::CCW_Rotation> : public Animation {
public:
	const Motion::CCW_Rotation &motion;

	virtual sf::Transform getTransform();

	Motion_Animation(const Motion::CCW_Rotation& M);
};
template<>
class Motion_Animation<Motion::Translation> : public Animation {
public:
	const Motion::Translation &motion;

	virtual sf::Transform getTransform();

	Motion_Animation(const Motion::Translation& M);

};




#endif // !ANIMATION_HPP
