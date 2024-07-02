#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP



class GameState {
	enum state {
		construction,
		simulation
	};

	state CurrentState;

public:
	const GameState& getState() const;

	void setup();


};








#endif // !GAMESTATE_HPP
