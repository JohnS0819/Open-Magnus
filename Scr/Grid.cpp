#include "Grid.hpp"
#include "Renderer.hpp"
#include <cmath>


template<>
void Rendering::Render_Window::draw(const Geometry::Grid& G) {
	auto& sprite = G.getSprite();
	RenderWindow::draw(sprite);

}


Geometry::Grid::Grid() {
	if (!gridTexture.loadFromFile("Complete_Lattice.png")) {
		std::cerr << "could not find grid texture file" << std::endl;
	}
	gridTexture.setSmooth(true);
	gridTexture.setRepeated(true);
	gridTexture.generateMipmap();
	grid.setTexture(gridTexture);
	grid.setPosition(0, 0);
	grid.setOrigin(591,512);
	grid.setScale(SCALEFACTOR, -SCALEFACTOR);
}

void Geometry::Grid::UpdateGridScaling(const sf::RenderWindow& window) {
	auto origin = grid.getOrigin();
	auto start = window.mapPixelToCoords({ 0,0 });
	auto end = window.mapPixelToCoords({ (int)window.getSize().x,(int)window.getSize().y });
	auto delta = end - start;
	double deltaX = delta.x;
	double deltaY = delta.y;
	double startX = start.x;
	double startY = start.y;

	sf::IntRect coords;
	coords.left = (start.x) / SCALEFACTOR;
	coords.top = std::fmodl((start.y) / SCALEFACTOR, 1023.64202727);
	coords.width = (delta.x + origin.x) / SCALEFACTOR;
	coords.height = (delta.y - origin.y) / SCALEFACTOR;
	grid.setTextureRect(coords);
	grid.setPosition(start.x,start.y);
	

}

const sf::Sprite& Geometry::Grid::getSprite() const {
	return grid;
}




Geometry::Grid& Geometry::Grid::instance() {
	static Grid singleton_;
	return singleton_;
}

float Geometry::constrain_angles(const float& angle) {
	float output = fmodf(angle, 6.28318530718);
	if (output < 0) {
		output += 6.28318530718;
	}
	return output;
}



std::ostream& Geometry::operator<<(std::ostream& os, const coordinate& point)
{
	os << "(" << point.x << ", " << point.y << ")";
	return os;
}

std::ostream& Geometry::operator<<(std::ostream& os, const grid_coordinate& point) {
	os << "(" << point.i << ", " << point.j << ")";
	return os;
}