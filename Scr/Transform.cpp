#include "Transforms.hpp"

Geometry::Rigid_Transform::Rigid_Transform() : Translation(0,0), Rotation(1,0) {}

Geometry::Rigid_Transform::Rigid_Transform(const Geometry::Coordinate_Transform& other) : Translation(other.delta) {
	Rotation.x = float(other.x_mat.i) + 0.5f * float(other.y_mat.i);
	Rotation.y = 0.866025403784f * float(other.y_mat.i);
}

void Geometry::Rigid_Transform::Apply(Geometry::coordinate& target) const {
	target.rotate(Rotation.x, Rotation.y);
	target.move_to(Translation);
}

void Geometry::Rigid_Transform::Commpose(const Geometry::Rigid_Transform& other) {
	float temp = Rotation.x * other.Rotation.x - Rotation.y * other.Rotation.y;
	float new_x1 = other.Rotation.x * Translation.x - other.Rotation.y * Translation.y;
	float new_y1 = other.Rotation.y * Translation.x + other.Rotation.x * Translation.y;
	
	Translation.x = new_x1 + other.Translation.x;
	Translation.y = new_y1 + other.Translation.y;
	Rotation.y = Rotation.x * other.Rotation.y + other.Rotation.x * Rotation.y;
	Rotation.x = temp;


}

Geometry::Rigid_Transform Geometry::Rigid_Transform::operator*(const Geometry::Rigid_Transform& other) const {
	Rigid_Transform output;
	output.Translation.x = other.Rotation.x * Translation.x - other.Rotation.y * Translation.y + other.Translation.x;
	output.Translation.y = other.Rotation.y * Translation.x + other.Rotation.x * Translation.y + other.Translation.y;
	output.Rotation.y = Rotation.x * other.Rotation.y + other.Rotation.x * Rotation.y;
	output.Rotation.x = Rotation.x * other.Rotation.x - Rotation.y * other.Rotation.y;
	return output;
}

Geometry::coordinate Geometry::Rigid_Transform::operator*(const Geometry::coordinate& other) const {
	coordinate output;
	output.x = Translation.x + other.x * Rotation.x - other.y * Rotation.y;
	output.y = Translation.y + other.x * Rotation.y + other.y * Rotation.x;
	return output;
}


Geometry::Coordinate_Transform::Coordinate_Transform(): delta({ 0,0 }), x_mat({ 1,0 }), y_mat({ 0,1 }) {}

void Geometry::Coordinate_Transform::Rotate() {
	y_mat.i += x_mat.i;
	y_mat.j += x_mat.j;
	x_mat.i -= y_mat.i;
	x_mat.j -= y_mat.j;
	delta.j += delta.i;
	delta.i -= delta.j;
}

void Geometry::Coordinate_Transform::counterRotate() {
	x_mat.i += y_mat.i;
	x_mat.j += y_mat.j;
	y_mat.i -= x_mat.i;
	y_mat.j -= x_mat.j;
	delta.i += delta.j;
	delta.j -= delta.i;
}

Geometry::Coordinate_Transform Geometry::Coordinate_Transform::inverse() const {
	Geometry::Coordinate_Transform output;
	output.x_mat.i = y_mat.j;
	output.x_mat.j = -x_mat.j;
	output.y_mat.i = -y_mat.i;
	output.y_mat.j = x_mat.i;
	output.delta.i = -output.x_mat.i * delta.i - output.x_mat.j * delta.j;
	output.delta.j = -output.y_mat.i * delta.i - output.y_mat.j * delta.j;
	return output;
}

void Geometry::Coordinate_Transform::translate(const Geometry::grid_coordinate& offset) {
	delta.i += offset.i;
	delta.j += offset.j;
}

Geometry::grid_coordinate Geometry::Coordinate_Transform::operator()(const Geometry::grid_coordinate& point) const {
	Geometry::grid_coordinate output;
	output.i = point.i * x_mat.i + point.j * x_mat.j + delta.i;
	output.j = point.i * y_mat.i + point.j * y_mat.j + delta.j;
	return output;
}

Geometry::grid_coordinate Geometry::Coordinate_Transform::ApplyRotation(const Geometry::grid_coordinate& point) const {
	Geometry::grid_coordinate output;
	output.i = point.i * x_mat.i + point.j * x_mat.j;
	output.j = point.i * y_mat.i + point.j * y_mat.j;
	return output;
}








Geometry::grid_coordinate Geometry::closestHexagon(const Geometry::coordinate& point) {
	coordinate transformed = { point.x - 0.57735026919f * point.y,  1.73205080757f * point.y };
	grid_coordinate guess = { transformed.x,transformed.y };
	auto insideHexagon = [](const grid_coordinate& hex, const coordinate& pnt) {
		float x_center = hex.i + hex.j * 0.5f;
		float y_center = hex.j * 0.866025403784f;
		return 2.0f * std::max(0.5f * fabs(x_center - pnt.x), 0.866025403784f * fabs(y_center - pnt.y)) + fabs(x_center - pnt.x) < 1.0f;
	};

	constexpr grid_coordinate deltas[6] = { {1,0},{0,1},{1,-1},{-1,1},{-1,0},{0,-1} };

	for (auto& i : deltas) {
		if (insideHexagon(guess + i, point)) {
			return guess + i;
		}
	}
	return guess;

}

Geometry::Coordinate_Transform Geometry::Coordinate_Transform::operator*(const Geometry::Coordinate_Transform& other) const {
	Geometry::grid_coordinate new_x;
	Geometry::grid_coordinate new_y;
	new_x.i = x_mat.i * other.x_mat.i + x_mat.j * other.y_mat.i;
	new_x.j = x_mat.i * other.x_mat.j + x_mat.j * other.y_mat.j;
	
	new_y.i = y_mat.i * other.x_mat.i + y_mat.j * other.y_mat.i;
	new_y.j = y_mat.i * other.x_mat.j + y_mat.j * other.y_mat.j;

	Geometry::grid_coordinate new_del;
	new_del.i = delta.i + other.delta.i * x_mat.i + other.delta.j * x_mat.j;
	new_del.j = delta.j + other.delta.i * y_mat.i + other.delta.j * y_mat.j;

	Geometry::Coordinate_Transform output;
	output.x_mat = new_x;
	output.y_mat = new_y;
	output.delta = new_del;
	return output;


}