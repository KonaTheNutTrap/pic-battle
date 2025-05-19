#ifndef AISYSTEM_H
#define AISYSTEM_H

#include "Character.h" // Needs full definition for Character&
#include "PassiveSystem.h"
#include <vector>
#include <string> 


enum class AIDifficulty {
    EASY,
    HARD
};

class AISystem {
public:
    static int chooseMove(const Character& botCharacter, const Character& playerCharacter, AIDifficulty difficulty);

private:
    struct MoveChoice {
        int move;
        double score;
        MoveChoice(int m, double s) : move(m), score(s) {}
    };

    static double scoreMoveHard(int botMove, const Character& bot, const Character& player);
    static double evaluatePassiveOutcome(const Passive& passive, const Character& self, const Character& opponent, bool selfIsActor);
    static int chooseMoveEasy(const Character& botCharacter, const Character& playerCharacter);
};

#endif // AISYSTEM_H