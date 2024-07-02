#include "Molecule.hpp"
#include "Renderer.hpp"
#include "Mechanism.hpp"
#include <algorithm>
#include "Tile.hpp"
#include "Connections.hpp"
using namespace Geometry;
Rendering::Atom_Primitive::Atom_Primitive(const Geometry::grid_coordinate& coord) {
	sf::Color color;
	/*if (coord == Geometry::grid_coordinate({0,0})) {
		color = sf::Color::Blue;

	}
	else {
		color = sf::Color::Red;
	}*/
	color = sf::Color::Red;
	for (int i = 0; i < NUMBER_OF_CIRCLE_POINTS - 1; ++i) {
		vertices[3 * i].position.x = COORDS_.points[2 * i] + (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i].position.y = COORDS_.points[2 * i + 1] + (0.866025403784f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i].color = color;

		vertices[3 * i + 1].position.x = COORDS_.points[2 * i + 2] + (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i + 1].position.y = COORDS_.points[2 * i + 3] + (0.866025403784f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i + 1].color = vertices[3 * i + 2].color = color;

		vertices[3 * i + 2].position.x = (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i + 2].position.y = (0.866025403784f * SPACE_TO_PIXELS * coord.j);
		vertices[3 * i + 2].color = color;
	}
	int n = NUMBER_OF_CIRCLE_POINTS - 1;
	vertices[3 * n].position.x = COORDS_.points[2 * n] + (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n].position.y = COORDS_.points[2 * n + 1] + (0.866025403784f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n].color = color;

	vertices[3 * n + 1].position.x = COORDS_.points[0] + (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n + 1].position.y = COORDS_.points[1] + (0.866025403784f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n + 1].color = color;

	vertices[3 * n + 2].position.x = (SPACE_TO_PIXELS * coord.i + 0.5f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n + 2].position.y = (0.866025403784f * SPACE_TO_PIXELS * coord.j);
	vertices[3 * n + 2].color = color;

}

Rendering::Atom_Primitive::Atom_Primitive() {
	for (int i = 0; i < NUMBER_OF_CIRCLE_POINTS -  1; ++i) {
		vertices[3 * i].position.x = COORDS_.points[2 * i];
		vertices[3 * i].position.y = COORDS_.points[2 * i + 1];
		vertices[3 * i].color = sf::Color::Red;

		vertices[3 * i + 1].position.x = COORDS_.points[2 * i + 2];
		vertices[3 * i + 1].position.y = COORDS_.points[2 * i + 3];
		vertices[3 * i + 1].color = vertices[3 * i + 2].color = sf::Color::Red;

		vertices[3 * i + 2].position.x = 0.0f;
		vertices[3 * i + 2].position.y = 0.0f;
		vertices[3 * i + 2].color = sf::Color::Red;

	}

	int n = NUMBER_OF_CIRCLE_POINTS - 1;
	vertices[3 * n].position.x = COORDS_.points[2 * n];
	vertices[3 * n].position.y = COORDS_.points[2 * n + 1];
	vertices[3 * n].color = sf::Color::Red;

	vertices[3 * n + 1].position.x = COORDS_.points[0];
	vertices[3 * n + 1].position.y = COORDS_.points[1];
	vertices[3 * n + 1].color = sf::Color::Red;

	vertices[3 * n + 2].position.x = 0.0f;
	vertices[3 * n + 2].position.y = 0.0f;
	vertices[3 * n + 2].color = sf::Color::Red;

}

Rendering::Atom_Primitive::Atom_Primitive(const Rendering::Atom_Primitive& tgt) {
	memcpy(vertices, tgt.vertices, sizeof(sf::Vertex) * 3 * NUMBER_OF_CIRCLE_POINTS);
	sizeof(Atom_Primitive);
}

Rendering::Molecule_Primitive::Molecule_Primitive() : connections(sf::Lines,0) {}

template<>
void Rendering::Render_Window::draw(const Rendering::Molecule_Primitive& M) {
	
	sf::Transform T;
	auto& orientation = M.Orientation;
	float angle = 57.2957795131 * atan2f(orientation.Rotation.y,orientation.Rotation.x);
	T.translate(orientation.Translation.x * GAMESPACE_TO_RENDERSPACE, orientation.Translation.y * GAMESPACE_TO_RENDERSPACE);
	T.rotate(angle);
	

	for (auto& i : M.collisions) {
		RenderWindow::draw(i, M.SFTRANSFORM);
	}
	RenderWindow::draw(M.connections, M.SFTRANSFORM);
	sf::RenderStates state;
	//state.transform = T;
	state.transform = M.SFTRANSFORM;
	RenderWindow::draw(&M.atoms[0].vertices[0],3 * M.atoms.size() * NUMBER_OF_CIRCLE_POINTS,sf::Triangles, state);

}


bool Molecule::query(const Geometry::grid_coordinate& point) const {
	Geometry::grid_coordinate coord = inverse_map(point);
	return Colliders.query(coord);
}


int Molecule::query_internal(const Geometry::grid_coordinate& point) const {

	Geometry::grid_coordinate coord = inverse_map(point);
	if (Colliders.coord_map[0].find(coord) == Colliders.coord_map[0].end()) {
		return -1;
	}
	return Colliders.coord_map[0].at(coord);

}

void Molecule::resizeHierarchy(int newSize) {
	Colliders.coord_map.resize(newSize);
	Colliders.nodes.resize(newSize);
	available_.resize(newSize);

}


void Molecule::bubbleUpdate(int level, std::unordered_set<int>& updates, std::unordered_set<int> &buffer_ ) {
	//if (level >= Colliders.nodes.size()) return;
	

	if (Colliders.coord_map.size() - 1 == level) {
		resizeHierarchy(level + 2);
	}
	if (Colliders.coord_map[level + 1].size() == 1) {
		 
		auto& problemNode = *Colliders.coord_map.back().begin();
		//std::cout << "detected problematic node " << problemNode.second << " at level " << level + 1 << std::endl;
		buffer_.insert(problemNode.second);
	}

	auto& current = Colliders.nodes[level];
	for (auto& index : updates) {
		//std::cout << "updated node " << index << "\t at level " << level << std::endl;
		auto updatedCollider = Colliders.updateCollider(level, index);
		auto parent = Graph::Collision_Hierarchy::findParent(current[index].coord);
		if (updatedCollider.radius_ < 0.0f) {
			if (Colliders.coord_map[level + 1].find(parent.first) == Colliders.coord_map[level + 1].end()) {
				std::cerr << "what" << std::endl;
			}
			int pos = Colliders.coord_map[level + 1][parent.first];
			//std::cout << "removed node " << index << "\t at position " << current[index].coord << "\t Adding node " << pos << " to update" << std::endl;
			Colliders.nodes[level + 1][pos].children[parent.second] = -1;
			buffer_.insert(pos);
			Colliders.coord_map[level].erase(current[index].coord);
			available_[level].push_back(index);

		}
		else {
			current[index].collision = updatedCollider;
			if (Colliders.coord_map[level + 1].find(parent.first) != Colliders.coord_map[level + 1].end()) {
				int pos = Colliders.coord_map[level + 1][parent.first];
				Colliders.nodes[level + 1][pos].children[parent.second] = index;
				buffer_.insert(Colliders.coord_map[level + 1][parent.first]);
				//std::cout << "updated node " << index << "\t at position " << current[index].coord << "\t Adding existing node " << pos << " to update" << std::endl;
			}
			else if (available_[level + 1].empty()) {
				int pos = Colliders.nodes[level + 1].size();
				buffer_.insert(pos);
				Colliders.coord_map[level + 1][parent.first] = pos;
				Colliders.nodes[level + 1].push_back(parent.first);
				Colliders.nodes[level + 1][pos].children[parent.second] = index;

				//std::cout << "updated node " << index << "\t at position " << current[index].coord << "\t Adding unused node " << pos << " to update" << std::endl;
			}
			else {
				int pos = available_[level + 1].back();
				available_[level + 1].pop_back();
				Colliders.coord_map[level + 1][parent.first] = pos;
				Colliders.nodes[level + 1][pos].coord = parent.first;
				Colliders.nodes[level + 1][pos].children[parent.second] = index;
				buffer_.insert(pos);
				//std::cout << "updated node " << index << "\t at position " << current[index].coord << "\t creating new node " << pos << " to update" << std::endl;

			}
		}

	}

	if (Colliders.coord_map[level].size() == 1) {
		//std::cout << "coordMap at level " << level << " is complete, resizing to " << level + 1 << std::endl;
		resizeHierarchy(level + 1);
		return;
	}
	updates.clear();
	bubbleUpdate(level + 1, buffer_, updates);


}

void Molecule::reset() {
	start_of_update_ = available_[0].size();
	componentMap.resize(0);
	for (auto& i : Colliders.coord_map[0]) {
		Colliders.nodes[0][i.second].children[0] = -1;
	}

}

void Molecule::rotate(const Geometry::grid_coordinate& pivot) {
	coordTransform.translate({ -pivot.i,-pivot.j });
	coordTransform.Rotate();
	coordTransform.translate(pivot);
	inverse_map = coordTransform.inverse();
	Orientation = coordTransform;
}

void Molecule::counterRotate(const Geometry::grid_coordinate& pivot) {
	coordTransform.translate({ -pivot.i,-pivot.j });
	coordTransform.counterRotate();
	coordTransform.translate(pivot);
	inverse_map = coordTransform.inverse();
	Orientation = coordTransform;
}



void Molecule::translate(const Geometry::grid_coordinate& delta) {
	coordTransform.translate(delta);
	inverse_map = coordTransform.inverse();
	Orientation = coordTransform;
}

void Molecule::clearMotion() {
	motionState = nullptr;
}

Molecule::Molecule() {
	start_of_update_ = 0;
	Colliders.coord_map.resize(1);
	Colliders.nodes.resize(1);
	available_.resize(1);
}


Molecule::Molecule(Graph::Collision_Hierarchy&& collision, Geometry::Coordinate_Transform T) :
	Colliders(std::move(collision)),
	coordTransform(T),
	inverse_map(T.inverse()),
	Orientation(T),
	start_of_update_(0)
{
	available_.resize(Colliders.coord_map.size());
}

Molecule::Molecule(Graph::Collision_Hierarchy&& collision) :
	Colliders(std::move(collision)),
	start_of_update_(0)

{
	available_.resize(Colliders.coord_map.size());

}

bool Molecule::setMotion(const Motion::Motion_Type* motion, const Geometry::grid_coordinate& MotionData) {
	if (motionState == nullptr || motionState->ccw_state.id == Motion::Motion_Type::ids_::nothing) {
		motionState = reinterpret_cast<const MotionUnion*> (motion);
		motion_data_ = MotionData;
		return true;
	}
	if (motionState->ccw_state.id == Motion::Motion_Type::ids_::stationary) {
		return motionState->ccw_state.id == motion->id;
	}
	static_assert(offsetof(Molecule::MotionUnion, ccw_state.pivot_) == offsetof(Motion::CCW_Rotation, pivot_));
	//const Motion::CCW_Rotation* casted = reinterpret_cast<const Motion::CCW_Rotation*>(motion);
	//return motionState->ccw_state.id == motion->id && motionState->ccw_state.pivot_ == casted->pivot_;
	return motionState->ccw_state.id == motion->id && MotionData == motion_data_;



}

//void Molecule::setMotionData(const Geometry::grid_coordinate& data) {
//	motion_data_ = data;
//	
//}



//bool Molecule::setMotion(const Motion::CCW_Rotation& motion) {
//	if (motionState.ccw_state.id == Motion::Motion_Type::ids_::nothing) {
//		memcpy(&motionState, &motion, sizeof(MotionUnion));
//		return true;
//	}
//	return motionState.ccw_state == motion;
//}
//
//bool Molecule::setMotion(const Motion::Translation& motion) {
//	if (motionState.translation_state.id == Motion::Motion_Type::ids_::nothing) {
//		memcpy(&motionState, &motion, sizeof(MotionUnion));
//		return true;
//	}
//	return motionState.translation_state == motion;
//}
//
//bool Molecule::setMotion(const Motion::Stationary& motion) {
//	if (motionState.stationary_state.id == Motion::Motion_Type::ids_::nothing) {
//		memcpy(&motionState, &motion, sizeof(MotionUnion));
//		return true;
//	}
//	return motionState.stationary_state.id == Motion::Motion_Type::ids_::stationary;
//}
//
//bool Molecule::setMotion(const Motion::CW_Rotation& motion) {
//
//	if (motionState.stationary_state.id == Motion::Motion_Type::ids_::nothing) {
//		memcpy(&motionState, &motion, sizeof(MotionUnion));
//		return true;
//	}
//	return motionState.cw_state == motion;
//
//}

void Molecule::updateOrientation() {
	if (motionState == nullptr) return;
	switch (this->motionState->ccw_state.id) {
	case (Motion::Motion_Type::ids_::ccw_rot):
		this->rotate(motion_data_);
		break;
	case (Motion::Motion_Type::ids_::cw_rot):
		this->counterRotate(motion_data_);
		break;
	case (Motion::Motion_Type::ids_::trans):
		this->translate(motion_data_);
		break;
	default:
		break;
	}
	motionState = nullptr;

}

void Molecule::recycle() {
	this->available_.resize(1);
	this->connectionCallback.clear();
	this->Colliders.nodes.resize(1);
	this->Colliders.coord_map.resize(1);
	this->Colliders.coord_map[0].clear();
	this->Colliders.nodes[0].resize(0);

	this->componentMap.clear();
	this->start_of_update_ = 0;
	this->Orientation = Rigid_Transform();
	this->inverse_map = Coordinate_Transform();
	this->coordTransform = Coordinate_Transform(); 



}


Geometry::Circle Molecule::getBoundingCircle() const {
	auto temp = Colliders.getBoundingCircle();
	this->Orientation.Apply(temp.center_);
	return temp; 
}

Geometry::Rectangle Molecule::getBoundingBox() const {
	auto circle = getBoundingCircle();
	Geometry::Rectangle output;
	output.LX = circle.center_.x - circle.radius_;
	output.LY = circle.center_.y - circle.radius_;
	output.RX = circle.center_.x + circle.radius_;
	output.UY = circle.center_.y + circle.radius_;
	return output;
}

Collider::rectangle Molecule::GenerateCollisionBox() const {
	
	auto obj = getBoundingCircle();
	
	if (motionState == nullptr) {
		Collider::rectangle output;
		output.LX = int(obj.center_.x - obj.radius_) - 1;
		output.UX = int(obj.center_.x + obj.radius_) + 1;
		output.LY = int(obj.center_.y - obj.radius_) - 1;
		output.UY = int(obj.center_.y + obj.radius_) + 1;
		return output;
	}
	const Motion::Motion_Type* casted = reinterpret_cast<const Motion::Motion_Type*>(motionState);
	return casted->Generate_Bounding_Box(obj);

	
	/*switch (motionState.ccw_state.id) {
	case(Motion::Motion_Type::ids_::stationary):
		output = motionState.stationary_state.Generate_Bounding_Box(getBoundingCircle());
		break;
	case(Motion::Motion_Type::ids_::ccw_rot):
		output = motionState.ccw_state.Generate_Bounding_Box(getBoundingCircle());
		break;
	case(Motion::Motion_Type::ids_::cw_rot):
		output = motionState.cw_state.Generate_Bounding_Box(getBoundingCircle());
		break;
	case(Motion::Motion_Type::ids_::trans):
		output = motionState.translation_state.Generate_Bounding_Box(getBoundingCircle());
		break;
	default:
		auto obj = getBoundingCircle();
		output.LX = int(obj.center_.x - obj.radius_) - 1;
		output.UX = int(obj.center_.x + obj.radius_) + 1;
		output.LY = int(obj.center_.y - obj.radius_) - 1;
		output.UY = int(obj.center_.y + obj.radius_) + 1;
	}
	return output;*/


}







MoleculeManager::MoleculeManager() {}



MoleculeManager& MoleculeManager::instance() {
	static MoleculeManager singleton_;
	return singleton_;

}

void MoleculeManager::delayDestruction(int id) {
	//markedForDestruction[id] = false;
}

Molecule* MoleculeManager::query(const Geometry::grid_coordinate& coord) const {
	for (auto& i : molecules_) {
		if (i->Colliders.query(coord)) {
			return i;
		}
		
	}
	return nullptr;

}

sf::Transform Molecule::getRenderTransform() const {
	double InterpolationFactor = Time_Singleton::instance().getInterpolation();
	const Motion::Motion_Type* casted = reinterpret_cast<const Motion::Motion_Type*>(motionState);
	Geometry::Rigid_Transform animationTransform;
	if (casted != nullptr) {
		animationTransform = casted->getTransform(InterpolationFactor);
	}
	Geometry::Rigid_Transform combinedTransform = Orientation * animationTransform;
	sf::Transform output;
	output.translate({ GAMESPACE_TO_RENDERSPACE * combinedTransform.Translation.x, GAMESPACE_TO_RENDERSPACE * combinedTransform.Translation.y });
	output.rotate(57.2957795131 * atan2(combinedTransform.Rotation.y, combinedTransform.Rotation.x));
	
	return output;


}

Graph::Collision_Hierarchy& MoleculeManager::getHierarchy(Molecule& M) {
	return M.Colliders;
}

std::vector<int>& MoleculeManager::getComponentMap(Molecule&M) {
	return M.componentMap;
}

std::vector<std::vector<int>>& MoleculeManager::getAvailable(Molecule& M) {
	return M.available_;
}
Geometry::Coordinate_Transform& MoleculeManager::getTransform(Molecule& M) {
	return M.coordTransform;
}


int MoleculeManager::number() const {
	return molecules_.size();
}

void MoleculeManager::markForDestruction(int id) {
	pusher.push(molecules_[id]);
	indicesToRemove.push(id);
	molecules_[id] = nullptr; 
}


Molecule* MoleculeManager::getNewMolecule() {
	int id = popper.pop();
	if (id >= 0) {
		auto ptr_struct = popper.get(id);
		auto output = ptr_struct.ptr;
		output->recycle();
		ptr_struct.ptr = nullptr;
		return output;
	}
	return new Molecule;

}

void MoleculeManager::trimMoleculeList() {
	std::remove(molecules_.begin(), molecules_.end(), nullptr);
}


Rendering::Molecule_Primitive MoleculeManager::generateInternal(const Molecule* M) const {
	const Molecule& molecule = *M;
	Rendering::Molecule_Primitive output;
	output.Orientation = molecule.Orientation;
	output.ptr = M;
	int size = molecule.Colliders.coord_map.size();
	std::vector<sf::Color> colors({ sf::Color::Green,sf::Color::Red,sf::Color::Blue,sf::Color::Magenta,sf::Color::Cyan,sf::Color::White,sf::Color::Yellow });
	int mod = 7;
	auto& coord_map = molecule.Colliders.coord_map;
	auto& nodes = molecule.Colliders.nodes;
	for (int j = 1; j < size; ++j) {
		for (auto& i : coord_map[j]) {
			int position = i.second;
			auto& node = nodes[j][position];
			sf::CircleShape circle(GAMESPACE_TO_RENDERSPACE * node.collision.radius_);
			circle.setFillColor(sf::Color::Transparent);
			circle.setOutlineColor(colors[j % mod]);
			circle.setOutlineThickness(8 * j + 4); // Adjust outline thickness as needed
			circle.setPosition(GAMESPACE_TO_RENDERSPACE * (node.collision.center_.x - node.collision.radius_), GAMESPACE_TO_RENDERSPACE * (node.collision.center_.y - node.collision.radius_));
			output.collisions.push_back(circle);
		}
		
	}
	for (auto& i : coord_map[0]) {
		int position = i.second;
		output.atoms.push_back(nodes[0][position].coord);
		for (int j = 1; j < 7; ++j) {
			if (nodes[0][position].children[j] == -1) continue;
			auto& neighbor = nodes[0][nodes[0][position].children[j]];
			auto& current = nodes[0][position];
			sf::Vertex V1, V2;
			V1.position.x = GAMESPACE_TO_RENDERSPACE * current.collision.center_.x;
			V1.position.y = GAMESPACE_TO_RENDERSPACE * current.collision.center_.y;
			V2.position.x = GAMESPACE_TO_RENDERSPACE * neighbor.collision.center_.x;
			V2.position.y = GAMESPACE_TO_RENDERSPACE * neighbor.collision.center_.y;
			V1.color = sf::Color::White;
			V2.color = sf::Color::White;
			output.connections.append(V1);
			output.connections.append(V2);
		}
	}
	for (int i = 0; i < 3 * NUMBER_OF_CIRCLE_POINTS; ++i) {
		output.atoms[0].vertices[i].color = sf::Color::Blue;
	}
	return output;
}

std::vector<Rendering::Molecule_Primitive> MoleculeManager::generateRenderables() const {
	std::vector<Rendering::Molecule_Primitive> output;
	for (auto& i : molecules_) {
		output.push_back(generateInternal(i));
	}
	return output;
}

void MoleculeManager::insert(const Molecule& M) {
	molecules_.push_back(new Molecule(M));
}

void MoleculeManager::insert(Molecule *M) {
	molecules_.push_back(M);
}

void MoleculeManager::TESTING() {
	TileManager::instance().reset();
	for (int i = 0; i < molecules_.size(); ++i) {
		TileManager::instance().query(*molecules_[i], i);
		molecules_[i]->callbackConnections(i);
	}
	TileManager::instance().generateConnectivityMap();
	TileManager::instance().generatePartitions();
	TileManager::instance().generateTasks();
	int TaskNumber = TileManager::instance().getNumberOfTasks();
	int TrivialTaskNumber = TileManager::instance().getNumberOfTrivialTasks();
	for (int i = 0; i < TaskNumber; ++i) {
		TileManager::instance().configureTask(i);
	}
	for (int i = 0; i < TrivialTaskNumber; ++i) {
		TileManager::instance().configureTrivialTask(i);
	}
	
	auto& connectMap = TileManager::instance().connectivityMap;
	ConnectionManager::instance().setupBuffers();
	for (int i = 0; i < connectMap.size(); ++i) {
		if (connectMap[i] == -1) continue;
		ConnectionManager::instance().setoffsets(i,connectMap[i]);
	}
	for (int i = 0; i < TileManager::instance().tasks.size(); ++i) {
		ConnectionManager::instance().setBufferSize(i);
	}
	std::vector<int> toDelete;
	for (int i = 0; i < connectMap.size(); ++i) {
		if (connectMap[i] == -1) continue;
		ConnectionManager::instance().extractAndCompress(*molecules_[i], i);
		if (TileManager::instance().moleculeOwners[i] == -1) {
			toDelete.push_back(i);
		}
	}
	for (int i = 0; i < TileManager::instance().partitionLinks.size(); ++i) {
		if (TileManager::instance().partitionLinks[i] != -1) {
			ConnectionManager::instance().update(i, *molecules_[TileManager::instance().partitionLinks[i]]);
		}
		else {
			Molecule* newMolecule = new Molecule();
			molecules_.push_back(newMolecule);
			ConnectionManager::instance().update(i, *newMolecule);
		}
	}

	auto& trivialTasks = TileManager::instance().getTrivialTasks();
	for (auto& i : trivialTasks) {
		ConnectionManager::instance().completeTrivial(*molecules_[i], i);
	}

	for (int i = toDelete.size() - 1; i >= 0; --i) {
		molecules_.erase(molecules_.begin() + toDelete[i]);
	}
	TileManager::instance().reset();



	

	for (auto& i : molecules_) {
		i->reset();
	}

	for (int i = 0; i < molecules_.size(); ++i) {
		TileManager::instance().query(*molecules_[i], i);
	}

	for (auto& i : molecules_) {
		i->reset();
	}



}

std::vector<Molecule*>& MoleculeManager::TESTUTIL() {
	return molecules_;
}