#include "Time.hpp"


Time_Singleton& Time_Singleton::instance() {
	static Time_Singleton singleton_;
	return singleton_;
}

Time_Singleton::Time_Singleton() : 
	animation_begin_(std::chrono::high_resolution_clock::now()),
	current_frame_(std::chrono::high_resolution_clock::now()),
	interval_(1.0),
	frame_fraction(0.0)
{}

void Time_Singleton::updateTime() {
	current_frame_ = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> current_interval = current_frame_ - animation_begin_;
	if (current_interval.count() > interval_.count()) {
		frame_fraction = 1.0;
	}
	else {
		frame_fraction = current_interval.count() / interval_.count();
	}
}

void Time_Singleton::nextFrame() {
	animation_begin_ = std::chrono::high_resolution_clock::now();
	current_frame_ = animation_begin_;
	frame_fraction = 0.0;

}


double Time_Singleton::getInterpolation() const {
	return frame_fraction;
}

void Time_Singleton::setAnimationDuration(const std::chrono::duration<double>& interval) {
	interval_ = interval;
}

bool Time_Singleton::ready() const {
	return frame_fraction >= 1.0f;
}