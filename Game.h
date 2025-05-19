#ifndef GAME_H
#define GAME_H

#include "Character.h"
#include "AISystem.h" 
#include <string>
#include <vector>
#include <cstdlib> 

class Game {
private:
    Character* player;
    Character* bot;
    bool debugMode;
    AIDifficulty currentAIDifficulty;
    int lastPlayerMove;
    int lastBotMove;    

    void displayHealth() const;
    int getRPSWinner(int playerMove, int botMove) const;
    std::string getMoveString(int move) const;
    Character* selectCharacter(const std::string& prompt);
    void resetBattleHistory(); 

public:
    Game();
    ~Game();

    void setDebugMode(bool debug);
    void setAIDifficulty(AIDifficulty difficulty);
    AIDifficulty getAIDifficulty() const;
    bool initialize();
    bool initializeDebug();
    void playRound();
    bool isGameOver() const;
    void announceWinner() const;
    void play();
};

#endif // GAME_H