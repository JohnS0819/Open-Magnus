#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP
#include <vector>
#include "Motion.hpp"
#include "Grid.hpp"
#include <array>
#include <memory>
namespace Instructions {
	enum Type {
		None = 0,
		Grab,
		Release,
		Rotate_CCW,
		Rotate_CW,
		Product_CCW,
		Product_CW,
		Translate_Pos,
		Translate_Neg,
		Extend,
		Retract
	};


	//Base class for Instructions, Internal Use only
	class GenericInstruction{
	protected:
		Type ID;

		std::array<char, sizeof(Motion::CCW_Rotation)> DATA_;

		const Geometry::grid_coordinate* MotionData_;
		Motion::Motion_Type *motion_;
	public:
		GenericInstruction(const Geometry::grid_coordinate& C1) : MotionData_(&C1) {
		}
		GenericInstruction() {}
		GenericInstruction(const GenericInstruction& target) : MotionData_(target.MotionData_){}

		void setDataSource(const Geometry::grid_coordinate* newMotionData) {
			MotionData_ = newMotionData;
		}


		void updateMotion() {
			Motion::CCW_Rotation* casted = reinterpret_cast<Motion::CCW_Rotation*>(motion_);
			casted->pivot_ = Geometry::coordinate(*MotionData_);

			std::cerr << casted->pivot_ << std::endl;
		}

		const Motion::Motion_Type* getMotion() const {
			return motion_;
		}

		Type getInstructionID() const {
			return ID;
		}

		Geometry::grid_coordinate getMotionData() const {
			return *MotionData_;
		}
	};



	template<Type T>
	class Instruction  : public GenericInstruction{

		using GenericInstruction::ID;
	public:
		Instruction(const Geometry::grid_coordinate& C1) : GenericInstruction(C1), ID(T) {

			static_assert(sizeof(Instruction<T>) == sizeof(GenericInstruction));

			if constexpr (T == Type::None || T == Type::Release || T == Type::Grab) {
				this->motion_ = new(DATA_.data()) Motion::Stationary;
			}
			if constexpr (T == Type::Rotate_CCW || T == Type::Product_CCW) {
				this->motion_ = new(DATA_.data()) Motion::CCW_Rotation;
			}
			if constexpr (T == Type::Rotate_CW || T == Type::Product_CW) {
				this->motion_ = new(DATA_.data()) Motion::CW_Rotation;
			}
			if constexpr (T == Type::Extend || T == Type::Retract || T == Type::Translate_Neg || T == Type::Translate_Pos) {
				this->motion_ = new(DATA_.data()) Motion::Translation;
			}
		}

		Instruction() : GenericInstruction() {
			ID = T;
			if constexpr (T == Type::None || T == Type::Release || T == Type::Grab) {
				this->motion_ = new(DATA_.data()) Motion::Stationary;
			}
			if constexpr (T == Type::Rotate_CCW || T == Type::Product_CCW) {
				this->motion_ = new(DATA_.data()) Motion::CCW_Rotation;
			}
			if constexpr (T == Type::Rotate_CW || T == Type::Product_CW) {
				this->motion_ = new(DATA_.data()) Motion::CW_Rotation;
			}
			if constexpr (T == Type::Extend || T == Type::Retract || T == Type::Translate_Neg || T == Type::Translate_Pos) {
				this->motion_ = new(DATA_.data()) Motion::Translation;
			}

		}

		Instruction(const Instruction& other) : GenericInstruction(other), ID(T) {
			if constexpr (T == Type::None || T == Type::Release || T == Type::Grab) {
				this->motion_ = new(DATA_.data()) Motion::Stationary;
			}
			if constexpr (T == Type::Rotate_CCW || T == Type::Product_CCW) {
				this->motion_ = new(DATA_.data()) Motion::CCW_Rotation;
			}
			if constexpr (T == Type::Rotate_CW || T == Type::Product_CW) {
				this->motion_ = new(DATA_.data()) Motion::CW_Rotation;
			}
			if constexpr (T == Type::Extend || T == Type::Retract || T == Type::Translate_Neg || T == Type::Translate_Pos) {
				this->motion_ = new(DATA_.data()) Motion::Translation;
			}		
		}
		
	};


	class Instruction_Tape {
	private:
		int position = 0;
		
	public:
		std::vector<std::shared_ptr<GenericInstruction>> Instructions_;

		Instruction_Tape(std::vector<std::shared_ptr<GenericInstruction>>&& other) : Instructions_(std::move(other)) {}
		
		Instruction_Tape() = default;
		Instruction_Tape(const Instruction_Tape&) = default;

		GenericInstruction* getNext() {
			GenericInstruction* output = Instructions_[position].get();
			++position;
			position %= Instructions_.size();
			output->updateMotion();
			return output;
		}




	};



}




#endif // !
