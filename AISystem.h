#ifndef AISYSTEM_H
#define AISYSTEM_H

#include "Character.h" 
#include "PassiveSystem.h"
#include <vector>
#include <string>

enum class AIDifficulty {
    EASY,
    HARD
};

class AISystem {
public:
    // Main function to be called by the game to get the bot's move
    // previousBotMove and previousPlayerMove default to 0 if not provided (e.g., for the first turn)
    static int chooseMove(const Character& botCharacter, const Character& playerCharacter, AIDifficulty difficulty, int previousBotMove = 0, int previousPlayerMove = 0);

private:
    // Structure to hold a move and its calculated score
    struct MoveChoice {
        int move; // 1=Rock, 2=Paper, 3=Scissors
        double score;
        std::string debugReasoning; // Optional: For logging why a move was scored high
        MoveChoice(int m, double s, std::string r = "") : move(m), score(s), debugReasoning(std::move(r)) {}
    };

    // --- Hard AI Helper Methods ---
    // Scores all possible bot moves against player context
    static std::vector<MoveChoice> scoreMovesHard(const Character& bot, const Character& player, int previousBotMove, int previousPlayerMove);
    // Evaluates a single bot move against a single player response
    static double evaluateSingleScenario(int botMove, int playerResponseMove, const Character& bot, const Character& player);
    // Evaluates the impact of a passive triggering
    static double evaluatePassiveOutcome(const Passive& passive, const Character& self, const Character& opponent, bool selfIsActor, const Character& originalBotState, const Character& originalPlayerState);
    // Selects a move based on scores using a probabilistic approach
    static int selectMoveFromScores(const std::vector<MoveChoice>& scoredMoves, const Character& bot, const Character& player);


    // --- Easy AI Helper Method ---
    // Determines the bot's move using simpler logic
    static int chooseMoveEasy(const Character& botCharacter, const Character& playerCharacter, int previousBotMove);

    // --- Utility ---
    static int getWinningMoveAgainst(int move); // e.g., Paper wins against Rock
    static int getLosingMoveAgainst(int move);  // e.g., Scissors loses against Rock
    static int getTyingMove(int move);          // e.g., Rock ties Rock
};

#endif // AISYSTEM_H