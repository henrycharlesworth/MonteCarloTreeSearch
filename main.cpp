#include "MCTS.h";
#include <iostream>
#include <vector>
#include <math.h>
#include <random>

//set up RNG.
std::random_device rd; //automatically seeds the RNG apparently!
std::mt19937 rng(rd()); //random number engine.

//TIC TAC TOE game.

//state class.
struct gameState {
	int plyrsGo; //0 or 1 (correspond to 1 and 2 on the actual board)
	std::vector<std::vector<int>> board; //board. 0 represents free, 1 represents x, 2 represents o.
	gameState() : board(3, std::vector<int>(3)) {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				board[i][j] = 0;
			}
		}
		plyrsGo = 0;
	};
	void printState() {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				std::cout << board[i][j] << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
};

//game action - just two indices.
struct gameAction {
	int ind1; int ind2;
	gameAction(int i1, int i2) {
		ind1 = i1; ind2 = i2;
	}
	gameAction() {
		ind1 = 0; ind2 = 0;
	}
};

std::vector<gameAction> getActions(gameState &cState) {
	std::vector<gameAction> actions;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (cState.board[i][j] == 0) {
				gameAction A(i, j);
				actions.push_back(A);
			}
		}
	}
	return actions;
}

gameState getNextState(gameState &cState, gameAction &action) {
	int i = action.ind1, j = action.ind2;
	gameState nextState = cState;
	int value = cState.plyrsGo + 1;
	nextState.board[i][j] = value;
	if (cState.plyrsGo == 0) {
		nextState.plyrsGo = 1;
	}
	else {
		nextState.plyrsGo = 0;
	}
	return nextState;
}

//may have to pass getNextState in as a function object??
gameState defaultPolicy(gameState &cState, std::vector<gameAction> &possibleActions) {
	std::uniform_int_distribution<int> uni(0, possibleActions.size() - 1);
	int index = uni(rng);
	return getNextState(cState, possibleActions[index]);
}

double selectionEvaluation(double totReward, int nj, int nTot) {
	//Implement UCB1
	return (totReward / nj) + sqrt(2 * log(nTot) / nj);
}

//check whether game is finished.
bool isTerminal(gameState &S) {
	int test = 1;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			test *= S.board[i][j];
		}
	}
	
	if (test > 0) return true; //no empty spaces left so game is definitely over.
		
	bool finished = false;
	//check rows
	for (int i = 0; i < 3; i++) {
		if (S.board[i][0] == S.board[i][1] && S.board[i][1] == S.board[i][2] && S.board[i][0] > 0) return true;
	}
	//check columns
	for (int i = 0; i < 3; i++) {
		if (S.board[0][i] == S.board[1][i] && S.board[1][i] == S.board[2][i] && S.board[0][i] > 0) return true;
	}
	//check diagonal 1
	if (S.board[0][0] == S.board[1][1] && S.board[1][1] == S.board[2][2] && S.board[0][0] > 0) return true;
	//check diagonal 2
	if (S.board[0][2] == S.board[1][1] && S.board[1][1] == S.board[2][0] && S.board[0][2] > 0) return true;

	return finished;
	
}

//assign rewards for each player based on terminal state.
std::vector<double> assignRewards(gameState &S) {
	std::vector<double> rewards(2);
	int winner, loser;
	for (int i = 0; i < 3; i++) {
		//rows
		if (S.board[i][0] == S.board[i][1] && S.board[i][0] == S.board[i][2] && S.board[i][0] > 0) {
			winner = S.board[i][1] - 1;
			loser = winner == 0 ? 1 : 0;
			rewards[winner] = 1.0; rewards[loser] = -1.0;
			return rewards;
		}
		//columns
		if (S.board[0][i] == S.board[1][i] && S.board[1][i] == S.board[2][i] && S.board[0][i] > 0) {
			winner = S.board[1][i] - 1;
			loser = winner == 0 ? 1 : 0;
			rewards[winner] = 1.0; rewards[loser] = -1.0;
			return rewards;
		}
	}
	//diagonals
	if ((S.board[0][0] == S.board[1][1] && S.board[1][1] == S.board[2][2] && S.board[0][0] > 0) || (S.board[0][2] == S.board[1][1] && S.board[1][1] == S.board[2][0] && S.board[2][0] > 0)) {
		winner = S.board[1][1] - 1;
		loser = winner == 0 ? 1 : 0;
		rewards[winner] = 1.0; rewards[loser] = -1.0;
		return rewards;
	}
	//otherwise board is full but no one has won
	rewards[0] = 0.0; rewards[1] = 0.0;
	return rewards;
}

int main() {

	gameState currentState; //create blank state to start game.
	int move1, move2;
	
	while (!isTerminal(currentState)) {
		currentState.plyrsGo = 0;
		bool invalidMove = true;
		while (invalidMove) {
			std::cout << "Move index 1: " << std::endl;
			std::cin >> move1;
			std::cout << "Move index 2: " << std::endl;
			std::cin >> move2;
			if (currentState.board[move1][move2] == 0) {
				invalidMove = false;
			}
			else {
				std::cout << "Cannot make this move!" << std::endl;
			}
		}
		gameAction A(move1, move2);
		currentState = getNextState(currentState, A);
		currentState.printState();
		if (isTerminal(currentState)) break;

		//make computer play
		currentGame<gameState, gameAction> testGame(currentState, 2);
		testGame.getActions = &getActions;
		testGame.getNextState = &getNextState;
		testGame.defaultPolicy = &defaultPolicy;
		testGame.selectionEvaluation = &selectionEvaluation;
		testGame.isTerminal = &isTerminal;
		testGame.assignRewards = &assignRewards;
		testGame.initialize();
		testGame.playNGames(5000);
		gameAction B = testGame.bestInitialAction();
		currentState = getNextState(currentState, B);
		currentState.printState();
	}
	
	
	return 0;

}