#ifndef GAUNTLETGAME_H
#define GAUNTLETGAME_H

#include "Character.h"
#include <vector>
#include <string>
#include <memory> 

using namespace std;

class GauntletGame {
private:
    unique_ptr<Character> playerCharacter;
    vector<string> unlockedGauntletCharacters;
    vector<Character*> opponentPrototypes; 
    int winsInCurrentRun;

    const string GAUNTLET_UNLOCKS_FILE = "gauntlet_unlocks.txt";
    const int OPPONENTS_TO_BEAT = 5;

    // Helper methods
    void loadGauntletUnlocks();
    void saveGauntletUnlocks();
    bool selectPlayerForGauntlet();
    void generateOpponentOrder(vector<Character*>& currentOpponentList);
    bool runBattle(Character& player, Character& opponent); // Manages a single battle
    string getMoveString(int move) const; 
    int getRPSWinner(int playerMove, int botMove) const; 
    void displayBattleStatus(const Character& p1, const Character& p2) const;
    void attemptUnlockNextCharacter();
    Character* cloneCharacter(const Character& prototype);


public:
    GauntletGame();
    void play(); // Main entry point for the gauntlet mode
};

#endif // GAUNTLETGAME_H
