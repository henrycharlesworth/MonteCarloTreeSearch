// Somewhat general implementation of Monte Carlo Tree Search. Only for games that the rewards are all provided at the termination of the game (could easily be adapted though).
// Note: final rewards should have |r| <= 1. Need to provide a number of functions to define the game logic.

// What needs to be provided:
//		- A class for the game state. should include an integer denoting players go (0,1,...) in variable plyrsGo
//		- A class for the possible actions. (probably easiest to use an integer in many cases).
//		- A function getActions(state) (returns vector of actions available)
//		- A function getNextState(state, int)
//		- A function defaultPolicy(state, vector<int> actions)
//		- A function selectionEvaluation(double,int, int) that evaluates how we select "intelligently" (e.g. UCB1)
//		- A function isTerminal(state)
//		- A function assignRewards(state) that evaluates the terminal state and determines which player won.

//e.g. usage:
//			currentGame Game(state);
//			Game.initialize;
//			Game.playNGames(N);
//			A moveToMake = Game.bestInitialAction();
//			Game.cleanUp();
//			Game.playNGames(N); //play again from the next state that's actually been chosen.
#include <functional>
#include <vector>
#include <algorithm>
#include <math.h>
#include <random>

#ifndef MCTS_H
#define MCTS_H


template<class S, class A> struct node; //forward declaration

//this class is a link between states, i.e. from state S, if we take the action corresponding to this link we get to state S'.
template <class S, class A>
struct link {
	node<S,A> *parentState; //pointer to "parent" state
	node<S,A> *nextState; //pointer to next state.
	double totReward = 0; //stores the sum of rewards when visiting this link.
	int nTotal = 0; //total number of times this link has been used.
};


//node in the game tree.
template <class S, class A>
struct node {
	S gameState;
	node<S,A> *prevNode;
	int prevAction; //action index which led to this state.
	std::vector<A> availableActions;
	std::vector<link<S,A>> availableLinks;
	bool allActionsTried = false; // have all available actions from this node been tried.
	int nVisits = 0; //number of times this node has been visited.
};


//main game class
template <class S, class A>
struct currentGame {
	currentGame(S initState, int players); //constructor
	node<S,A> rootNode; //current node of the game, from which we will explore possible futures.
	node<S,A> *currentNode; //current node being considered.
	int nPlayers;
	std::vector<double> gameRewards; //stores the reward for each player.
	std::function< std::vector<A>(S &) > getActions; //return a vector of possible actions from the given state
	std::function< S(S &, A & ) > getNextState; //given a state S and an action A, return the resulting state.
	std::function< S(S &, std::vector<A> &) > defaultPolicy; //the "default" policy, i.e. what action is selected without using MCTS (usually random unless there is some "expert knowledge" that can be added).
	std::function< double(double, int, int) > selectionEvaluation; //provide a function for evaluating how to select moves (e.g. UCB1)
	std::function< bool(S &) > isTerminal; //check if state reached is terminal.
	std::function< std::vector<double>(S &) > assignRewards; //apply to a terminal state and return the reward for each player
	void initialize(); // set up available actions from root node etc.
	void selection(); //cycle through the game tree selecting actions "intelligently" for as long as we can.
	void expansion(); //once we can use selection no more, expand from this node.
	void finishSimulation(); //run simulation to end.
	void backpropagation(); //backpropagate the result back up the tree.
	void playNGames(int N); //run N games from the current root node.
	A bestInitialAction(); //returns the best current action from the root node.
	void deleteNode(node<S, A> *cNode); //delete node from memory.
	void cleanUp(); //deletes all of the tree which does not correspond to what's left after the best available current action.
	
};


//constructor for the main game.
template <class S, class A>
currentGame<S,A>::currentGame(S initState, int players) {
	nPlayers = players; //number of players
	rootNode.gameState = initState;
	rootNode.prevNode = NULL;
	rootNode.prevAction = NULL;
}


//initialize the root node (available actions and the states these lead to).
template <class S, class A>
void currentGame<S,A>::initialize() {
	rootNode.availableActions = getActions(rootNode.gameState);
	for (int i = 0; i < rootNode.availableActions.size(); i++) {
		link<S,A> newLink;
		newLink.parentState = &rootNode;
//		S nextState = getNextState(rootNode.gameState, rootNode.availableActions[i]);
		newLink.nextState = new node<S,A>;
//		newLink.nextState.gameState = nextState;
		rootNode.availableLinks.push_back(newLink);
	}
}



//run "selection" according to selectionEvaluation until we need to expand the tree.
template <class S, class A>
void currentGame<S,A>::selection() {
	currentNode = &rootNode; //start at the root.
	
	while (currentNode->allActionsTried) {
		double maxValue = -99999999999;
		int maxInd, counter = 0;
		for (auto & nextLink : currentNode->availableLinks) {
			double moveValue = selectionEvaluation(nextLink.totReward, nextLink.nTotal, nextLink.parentState->nVisits);
			if (moveValue > maxValue) {
				maxInd = counter;
				maxValue = moveValue;
			}
			counter++;
		}
		currentNode = currentNode->availableLinks[maxInd].nextState;
		if (isTerminal(currentNode->gameState)) break;
	}
}


// expansion - expand the game tree when we can no longer select intelligently.
template <class S, class A>
void currentGame<S,A>::expansion() {
	//check which actions haven't been tried yet from this state:
	std::vector<int> unTried;
	for (int i = 0; i < currentNode->availableLinks.size(); i++) {
		if (currentNode->availableLinks[i].nTotal == 0) {
			unTried.push_back(i);
		}
	}
	std::random_shuffle(unTried.begin(), unTried.end()); // randomly choose which untried action to expand.
	if (unTried.size() == 1) {
		//then we are trying to final untried action here.
		currentNode->allActionsTried = true;
	}
	int actionTaken = unTried[0]; //select action we are expanding on
	//update currentNode
	S newState = getNextState(currentNode->gameState, currentNode->availableActions[actionTaken]);
	auto newNode = currentNode->availableLinks[actionTaken].nextState;
	newNode->gameState = newState; // fill in new state.
	newNode->prevNode = currentNode;
	newNode->prevAction = actionTaken;
	newNode->availableActions = getActions(newState);
	for (int i = 0; i < newNode->availableActions.size(); i++) {
		link<S,A> newLink;
		newLink.parentState = newNode;
		newLink.nextState = new node<S,A>;
		newNode->availableLinks.push_back(newLink);
	}
	currentNode = newNode;
}


//run simulation to the end from current Node.
template <class S, class A>
void currentGame<S,A>::finishSimulation() {
	S currState = currentNode->gameState;
	while (!isTerminal(currState)) {
		//currState.printState(); //can be useful for debugging to have this function.
		currState = defaultPolicy(currState, getActions(currState));
	}
	gameRewards = assignRewards(currState);
}


//backpropagation phase
template <class S, class A>
void currentGame<S,A>::backpropagation() {
	//current node should still be the node after the expansion was made, and we backpropagate from here.
	while (currentNode->prevNode != NULL) {
		int pAction = currentNode->prevAction;
		currentNode = currentNode->prevNode;
		currentNode->nVisits++;
		currentNode->availableLinks[pAction].nTotal++;
		currentNode->availableLinks[pAction].totReward += gameRewards[currentNode->gameState.plyrsGo];
	}
}

//carry out the actual simulations:
template<class S, class A>
void currentGame<S,A>::playNGames(int N) {
	//should already be initialized.
	for (int i = 0; i < N; i++) {
		selection();
		if (!isTerminal(currentNode->gameState)) {
			expansion();
			finishSimulation();
		}
		else {
			gameRewards = assignRewards(currentNode->gameState);
		}
		backpropagation();
	}
}


//return best current move (from the root node, i.e. which initial move is best)
template <class S, class A>
A currentGame<S,A>::bestInitialAction() {
	double maxValue = -9999999999999.0;
	A maxMove;
	for (int i = 0; i < rootNode.availableLinks.size(); i++) {
		double moveValue = rootNode.availableLinks[i].totReward / ((double)rootNode.availableLinks[i].nTotal);
		//std::cout << "move " << i << ": " << moveValue << std::endl;
		if (moveValue > maxValue) {
			maxValue = moveValue;
			maxMove = rootNode.availableActions[i];
		}
	}
	std::cout << std::endl;
	return maxMove;
}

//clear node from memory, and all of the children nodes as well.
template <class S, class A>
void currentGame<S, A>::deleteNode(node<S, A> *cNode) {
	if (!isTerminal(cNode->gameState)) {
		for (int i = 0; i < cNode->availableLinks.size(); i++) {
			deleteNode(cNode->availableLinks.nextState);
		}
	}
	delete cNode;
}

//once a move has been chosen, clear the parts of the tree that are no longer in use and change the root node to the new state.
template <class S, class A>
void currentGame<S, A>::cleanUp() {
	double maxVal = -99999999999999.0; int maxInd;
	for (int i = 0; i < rootNode.availableLinks.size(); i++) {
		double val = rootNode.availableLinks[i].totReward / rootNode.availableLinks[i].nTotal;
		if (val > maxVal) {
			maxVal = val; maxInd = i;
		}
	}
	for (int i = 0; i < rootNode.availableLinks.size(); i++) {
		if (i == maxInd) continue;
		deleteNode(rootNode.availableLinks[i].nextState);
	}
	rootNode = *(rootNode.availableLinks[maxInd].nextState);
}


#endif