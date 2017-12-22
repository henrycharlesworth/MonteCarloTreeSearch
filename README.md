# MonteCarloTreeSearch
Somewhat general implementation of Monte Carlo Tree Search. Only for games that the rewards are all provided at the termination of the game (could easily be adapted though).
Note: final rewards should have |r| <= 1. Need to provide a number of functions to define the game logic.
I made this mainly to get some practice using the c++ standard template library so I made no real attempt to make the code particularly efficient.

What needs to be provided:
	- A class for the game state. should include an integer denoting players go (0,1,...) in variable plyrsGo
  - A class for the possible actions. (probably easiest to use an integer in many cases).
  - A function getActions(state) (returns vector of actions available)
  - A function getNextState(state, int)
  - A function defaultPolicy(state, vector<int> actions)
	- A function selectionEvaluation(double,int, int) that evaluates how we select "intelligently" (e.g. UCB1)
	- A function isTerminal(state)
	- A function assignRewards(state) that evaluates the terminal state and determines which player won.

e.g. usage:
		currentGame Game(state);
		Game.initialize;
		Game.playNGames(N);
		A moveToMake = Game.bestInitialAction();
		Game.cleanUp();
		Game.playNGames(N); //play again from the next state that's actually been chosen.
