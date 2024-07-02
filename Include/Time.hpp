#ifndef TIME_HPP
#define TIME_HPP
#include <chrono>


//NOT THREAD SAFE
class Time_Singleton {
private:
	std::chrono::high_resolution_clock::time_point animation_begin_;
	std::chrono::high_resolution_clock::time_point current_frame_;
	std::chrono::duration<double> interval_;
	double frame_fraction;

	Time_Singleton();
public:
	static Time_Singleton& instance();
	
	void nextFrame();
	
	void setAnimationDuration(const std::chrono::duration<double> &);

	void updateTime();

	double getInterpolation() const;

	bool ready() const;

};


#endif // !TIME_HPP