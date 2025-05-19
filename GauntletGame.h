#ifndef GAUNTLETGAME_H
#define GAUNTLETGAME_H

#include "Character.h"
#include "AISystem.h" // For AIDifficulty
#include <vector>
#include <string>
#include <memory>

class GauntletGame {
private:
    std::unique_ptr<Character> playerCharacter;
    std::vector<std::string> unlockedGauntletCharacters;
    int winsInCurrentRun;

    // For AI history within a single gauntlet battle
    int lastPlayerMoveInBattle;
    int lastBotMoveInBattle;


    const std::string GAUNTLET_UNLOCKS_FILE = "gauntlet_unlocks.txt";
    const int OPPONENTS_TO_BEAT = 5;

    void loadGauntletUnlocks();
    void saveGauntletUnlocks();
    bool selectPlayerForGauntlet();
    void generateOpponentOrder(std::vector<Character*>& currentOpponentList);
    bool runBattle(Character& player, Character& opponentProto);
    std::string getMoveString(int move) const;
    int getRPSWinner(int playerMove, int botMove) const;
    void displayBattleStatus(const Character& p1, const Character& p2) const;
    void attemptUnlockNextCharacter();
    void resetGauntletBattleHistory();


public:
    GauntletGame();
    void play();
};

#endif // GAUNTLETGAME_H