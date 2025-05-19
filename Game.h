#ifndef GAME_H
#define GAME_H

#include "Character.h" 
#include <string>      
#include <vector>     
#include <cstdlib>     


using namespace std;

class Game {
private:
    Character* player;
    Character* bot;
    bool debugMode;

    int getRandomMove() const;
    void displayHealth() const;
    int getRPSWinner(int playerMove, int botMove) const;
    string getMoveString(int move) const;
    Character* selectCharacter(const string& prompt);

public:
    Game();
    ~Game(); // Though default is fine with raw pointers to managed objects

    void setDebugMode(bool debug);
    bool initialize();
    bool initializeDebug();
    void playRound();
    bool isGameOver() const;
    void announceWinner() const;
    void play();
};

#endif // GAME_H
