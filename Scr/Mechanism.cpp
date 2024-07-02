#include "Mechanism.hpp"
#include "Molecule.hpp"
#include "Renderer.hpp"

Mechanism::Mechanism(std::vector<std::shared_ptr<Instructions::GenericInstruction>>&& other) : tape(std::move(other)), Coarse_Collidable(), Geometry::grid_coordinate() {}


void Mechanism::read() {
	for (auto& i : heads_) {
		i->updatePosition();
	}
	updateOrientation();

	currentInstruction = tape.getNext();

	for (auto& i : heads_) {
		i->updateQueryable();
	}

}


//const std::vector<std::unique_ptr<Mechanism::Head>>& Mechanism::getHeads() const {
//	return heads_;
//}

bool Mechanism::Head::isAttached() const {
	return attached_;
}

void Mechanism::updateOrientation() {

	if (currentInstruction == nullptr) return;
	Geometry::grid_coordinate& name = *this;
	Geometry::grid_coordinate data = currentInstruction->getMotionData();
	Geometry::Coordinate_Transform T;
	switch (currentInstruction->getMotion()->getId()) {
	case(Motion::Motion_Type::ids_::ccw_rot):
		T.translate({ -data.i,-data.j });
		T.Rotate();
		T.translate(data);
		break;
	case(Motion::Motion_Type::ids_::cw_rot):
		T.translate({ -data.i,-data.j });
		T.counterRotate();
		T.translate(data);
		break;
	case(Motion::Motion_Type::ids_::trans):
		T.translate(data);
		break;
	default:
		break;
	}

	name = T(name);
	return;



	return;
}



bool Mechanism::Head::attach(Molecule& M) {
	attached_ = true;
	return M.setMotion((*ptr_)->getMotion(), (*ptr_)->getMotionData());
}

bool Mechanism::Head::isQueryable() const {
	return queryable_;
}

void Mechanism::Head::updatePosition() {
	if (ptr_ == nullptr || (*ptr_) == nullptr) return;
	Geometry::grid_coordinate& name = *this;
	Geometry::grid_coordinate data = (*ptr_)->getMotionData();
	Geometry::Coordinate_Transform T;
	switch ((*ptr_)->getMotion()->getId()) {
	case(Motion::Motion_Type::ids_::ccw_rot):
		T.translate({-data.i,-data.j});
		T.Rotate();
		T.translate(data);
		break;
	case(Motion::Motion_Type::ids_::cw_rot):
		T.translate({ -data.i,-data.j });
		T.counterRotate();
		T.translate(data);
		break;
	case(Motion::Motion_Type::ids_::trans):
		T.translate(data);
		break;
	default:
		break;
	}

	name = T(name);
	return;

}


template<>
void Rendering::Render_Window::draw(const Mechanism::Head& head) {
	sf::CircleShape C;
	if (head.isAttached()) {
		C.setOutlineColor(sf::Color::Green);
	}
	else if (head.isQueryable()) {
		C.setOutlineColor(sf::Color::Cyan);
	}
	else {
		C.setOutlineColor(sf::Color::Blue);
	}
	C.setPointCount(20);
	C.setRadius(ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE);
	C.setOrigin({ ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE,ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE });
	Geometry::coordinate _coord = head;

	Geometry::Rigid_Transform Transform = head.getCurrentInstruction()->getMotion()->getTransform(Time_Singleton::instance().getInterpolation());
	//std::cerr << Transform.Translation << std::endl;
	Transform.Apply(_coord);
	C.setPosition(_coord.x * GAMESPACE_TO_RENDERSPACE, _coord.y * GAMESPACE_TO_RENDERSPACE);
	C.setOutlineThickness(15);
	C.setFillColor(sf::Color::Transparent);
	RenderWindow::draw(C);


}

template<>
void Rendering::Render_Window::draw(const Mechanism& mechanism) {
	sf::CircleShape C;
	C.setOutlineColor(sf::Color::Magenta);
	C.setPointCount(20);
	C.setRadius(ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE);
	C.setOrigin({ ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE,ATOMIC_RADIUS * GAMESPACE_TO_RENDERSPACE });
	Geometry::coordinate _coord = mechanism;

	Geometry::Rigid_Transform Transform = mechanism.currentInstruction->getMotion()->getTransform(Time_Singleton::instance().getInterpolation());
	//std::cerr << Transform.Translation << std::endl;
	Transform.Apply(_coord);
	C.setPosition(_coord.x * GAMESPACE_TO_RENDERSPACE, _coord.y * GAMESPACE_TO_RENDERSPACE);
	C.setOutlineThickness(15);
	C.setFillColor(sf::Color::Transparent);
	RenderWindow::draw(C);
	for (auto& i : mechanism.heads_) {
		draw(*i);
	}
}










void Mechanism::Head::updateQueryable() {
	if ((*ptr_)->getInstructionID() == Instructions::Type::Release) {
		attached_ = false;
		queryable_ = false;
		return;
	}
	if (attached_) {
		queryable_ = true;
		return;
	}
	if (state_ == HeadState::open && (*ptr_)->getInstructionID() == Instructions::Type::Grab) {
		queryable_ = true;
		return;
	}
	queryable_ = false;
}


bool Mechanism::intersect(Molecule& M) {
	bool result = true;
	for (auto& head : heads_) {
		if (!head->isQueryable()) {
			continue;
		}
		if (!M.query(*head)) {
			continue;
		}
		
		result &= head->attach(M);
	}
	return result;
}

Geometry::grid_coordinate Mechanism::Head::getMotionData() const {
	return (*ptr_)->getMotionData();
}


void Mechanism::updateHeads() {}

Geometry::grid_coordinate Mechanism::getPosition() const {
	return this->position;
}

const Instructions::GenericInstruction* Mechanism::Head::getCurrentInstruction() const {
	return (*ptr_);
}

Motion::Motion_Type::ids_ Mechanism::getMotionType() const {
	return motionType;
}

Geometry::Rectangle Mechanism::getBoundingBox() const {
	auto& center = Coarse_Collidable::Circle::center_;
	auto& radius = Coarse_Collidable::Circle::radius_;

	Geometry::Rectangle output;
	output.LX = center.x - radius;
	output.LY = center.y - radius;
	output.RX = center.x + radius;
	output.UY = center.y + radius;
	return output;
}

void MechanismManager::insert(Mechanism* M) {
	mechanisms_.push_back(M);
}

void MechanismManager::insert(const Mechanism& M) {
	mechanisms_.push_back(new Mechanism(M));
}

void MechanismManager::buildTree() {
	using namespace boost::geometry;
	std::vector<R_Tree::RectPair> constructor;
	for (int i = 0; i < mechanisms_.size(); ++i) {
		constructor.push_back(std::make_pair(convertToBoostFormat(mechanisms_[i]->getBoundingBox()), mechanisms_[i]));
	}
	tree_.reset();
	tree_ = std::make_unique<R_Tree>(R_Tree(constructor));
}

std::vector<Mechanism*> MechanismManager::query(const Molecule& M) const {
	return tree_->query(convertToBoostFormat( M.getBoundingBox()));
}
