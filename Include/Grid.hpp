#ifndef GRID_HPP
#define GRID_HPP

#include <cmath>
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>


namespace Geometry {

	float constrain_angles(const float& angle);
	constexpr int32_t SPACE_TO_PIXELS = 128;

	/*
	!@brief represents a coordinate in hexagonal space
	*/
	struct grid_coordinate {
		int i;
		int j;


		grid_coordinate operator+(const grid_coordinate& other) const {
			return { i + other.i, j + other.j };
		}

		void operator=(const grid_coordinate& target) {
			i = target.i;
			j = target.j;
		}

		bool operator==(const grid_coordinate& other) const {
			return i == other.i && j == other.j;
		}

		grid_coordinate operator-(const grid_coordinate& other) const {
			return { i - other.i,j - other.j };
		}


	};





	/*
	!@brief represents a point in cartesian space
	*/
	struct coordinate {
		float x;
		float y;

		static constexpr int DIM = 2;
		template<typename t>
		float& operator[](const t& index) {
			static_assert(offsetof(Geometry::coordinate, Geometry::coordinate::x) < offsetof(Geometry::coordinate, Geometry::coordinate::y));
			float* output = &x;
			return *(output + index);
		}

		template<typename t>
		float operator[](const t& index) const {
			static_assert(offsetof(Geometry::coordinate, Geometry::coordinate::x) < offsetof(Geometry::coordinate, Geometry::coordinate::y));
			const float* output = &x;
			return *(output + index);
		}

		void operator=(const coordinate& target) {
			x = target.x;
			y = target.y;
		}

		coordinate(const coordinate& copy) : x(copy.x), y(copy.y){}

		coordinate(const grid_coordinate& copy) : x(float(copy.i) + 0.5f * float(copy.j)), y(0.866025403784f * float(copy.j)) {}

		coordinate(float x_, float y_) : x(x_), y(y_)  {}

		coordinate() {
			x = 0.0;
			y = 0.0;
		}

		float distance_to(const coordinate& other) const {
			return std::sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
		}

		float distance_to_sq(const coordinate& other) const {
			return ((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
		}


		void move_to(const coordinate& other) {
			x += other.x;
			y += other.y;
		}

		template<typename T>
		void rotate(const T& angle) {
			float temp = cos(angle) * x - sin(angle) * y;
			y = sin(angle) * x + cos(angle) * y;
			x = temp;
		}
		template<typename T>
		void rotate(const T& x_0, const T& y_0) {
			float temp = x * x_0 - y * y_0;
			y = x * y_0 + y * x_0;
			x = temp;
		}
		template<typename T>
		void counter_rotate(const T& x_0, const T& y_0) {
			float temp = x * x_0 + y * y_0;
			y = y * x_0 - x * y_0;
			x = temp;
		}


		coordinate operator-(const coordinate& other) const {
			return coordinate(x - other.x, y - other.y);
		}

		coordinate operator+(const coordinate& other) const {
			return coordinate(x + other.x, y + other.y);
		}

		void operator+=(const coordinate& other){
			x += other.x;
			y += other.y;
		}

		void operator-=(const coordinate& other) {
			x -= other.x;
			y -= other.y;
		}

		template<typename T>
		coordinate operator/(const T& constant) const {
			return coordinate(x / constant, y / constant);
		}
		template<typename T>
		coordinate operator*(const T& constant) const {
			return coordinate(x * constant, y * constant);
		}

		template<typename T>
		void operator*=(const T& constant){
			x *= constant;
			y *= constant;
		}

		bool operator==(const coordinate& other) const {
			return x == other.x && y == other.y;
		}



	};

	std::ostream& operator<<(std::ostream& os, const grid_coordinate& point);

	std::ostream& operator<<(std::ostream& os, const coordinate& point);

	

	struct Circle {
		coordinate center_;
		float radius_;
	};


	struct Rectangle {
		float LX;
		float LY;
		float RX;
		float UY;
	};


	class Grid;

	



	class Grid {
	private:
		Grid();
		Grid(const Grid&) = delete;

		

	protected:

		static constexpr int GRID_X_DIMENSION = 1182;
		static constexpr int GRID_Y_DIMESNION = 1024;

		static constexpr double SCALEFACTOR = 0.216582064298;

		sf::Texture gridTexture;

		sf::Sprite grid;


	public:
		static Grid& instance();

		const sf::Sprite& getSprite() const;

		void UpdateGridScaling(const sf::RenderWindow&);

	};
}

template<>
struct std::hash<Geometry::grid_coordinate> {
	std::size_t operator()(const Geometry::grid_coordinate& k) const {
		std::hash<int> hasher;
		return hasher(k.i) ^ (hasher(k.j) >> 1);
	}

};
#endif // !GRID_H
