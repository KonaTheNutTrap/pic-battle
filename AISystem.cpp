#include "AISystem.h"
#include <vector>
#include <algorithm>    // For std::sort, std::shuffle, std::max_element
#include <random>       // For std::random_device, std::mt19937, std::discrete_distribution, std::uniform_int_distribution
#include <iostream>     // For std::cout, std::endl (debug only)
#include <map>          // For potential future use (not currently used)
#include <cmath>        // For std::abs, std::pow, std::exp
#include <sstream>      // For std::stringstream (for debugReasoning)
#include <utility>      // For std::move

// using namespace std; // OK in .cpp

namespace {
    // --- Scoring constants for HARD AI ---
    const double K_DAMAGE_DEALT_PER_HP = 1.5;
    const double K_LETHAL_BONUS = 200.0;
    const double K_DAMAGE_TAKEN_PER_HP = 1.8;
    const double K_DEATH_PENALTY = 250.0;
    const double K_TIE_OUTCOME_BASE = -0.5;
    const double K_MOVE_BASE_DAMAGE_BIAS = 0.2;

    // --- Passive evaluation multipliers ---
    const double K_PASSIVE_HEAL_MULT = 1.2;
    const double K_PASSIVE_DAMAGE_MULT = 1.3;
    const double K_PASSIVE_BUFF_MULT = 1.0;
    const double K_PASSIVE_PERM_BUFF_MULT = 2.0;

    // --- Risk/Reward Modifiers ---
    const double K_BOT_LOW_HP_THRESHOLD = 0.25;
    const double K_PLAYER_LOW_HP_THRESHOLD = 0.30;
    const double K_DESPERATION_HEAL_BONUS = 15.0;
    const double K_AGGRESSION_LETHAL_ATTEMPT_BONUS = 20.0;
    const double K_HP_LEAD_CONSERVATION_FACTOR = 0.8;

    // --- Move Variety ---
    const double K_REPEATED_MOVE_PENALTY = 5.0;

    // --- Probabilistic Choice (Softmax temperature) ---
    // K_SELECTION_TEMPERATURE: Higher for more randomness among top choices, Lower for more greediness.
    // 0.1 or less is very greedy. 1.0 is moderate. 2.0+ gets quite random.
    const double K_SELECTION_TEMPERATURE = 1.0; // Adjusted for more reasonable spread


    // Helper to get an estimated damage value for a character's move
    int getEstimatedDamage(const Character& c, int move) {
        int baseDmg = 0;
        switch (move) {
        case 1: baseDmg = c.getRockDamage(); break;
        case 2: baseDmg = c.getPaperDamage(); break;
        case 3: baseDmg = c.getScissorsDamage(); break;
        }
        return baseDmg + c.getBonusDamageNextAttack();
    }

    std::string moveToString(int move) {
        if (move == 1) return "Rock";
        if (move == 2) return "Paper";
        if (move == 3) return "Scissors";
        return "Unknown";
    }

} // end anonymous namespace


// --- Public Static Methods ---
int AISystem::chooseMove(const Character& botCharacter, const Character& playerCharacter, AIDifficulty difficulty, int previousBotMove, int previousPlayerMove) {
    if (difficulty == AIDifficulty::EASY) {
        return chooseMoveEasy(botCharacter, playerCharacter, previousBotMove);
    }
    else { // HARD difficulty
        std::vector<MoveChoice> scoredMoves = scoreMovesHard(botCharacter, playerCharacter, previousBotMove, previousPlayerMove);
        return selectMoveFromScores(scoredMoves, botCharacter, playerCharacter);
    }
}

// --- Easy AI ---
int AISystem::chooseMoveEasy(const Character& botCharacter, const Character& playerCharacter, int previousBotMove) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_100(1, 100);
    std::uniform_int_distribution<> distrib_move(1, 3);

    if (distrib_100(gen) <= 25) { // 25% chance of completely random
        return distrib_move(gen);
    }

    std::vector<MoveChoice> potentialMoves;
    bool playerVeryLowHp = (static_cast<double>(playerCharacter.getCurrentHp()) / playerCharacter.getMaxHp()) < K_PLAYER_LOW_HP_THRESHOLD;

    for (int m = 1; m <= 3; ++m) {
        double currentScore = 0;
        int damageOutput = getEstimatedDamage(botCharacter, m);

        currentScore += damageOutput * 0.7; // Base damage more important

        if (playerVeryLowHp && damageOutput > 0 && (playerCharacter.getCurrentHp() - damageOutput <= 0)) {
            currentScore += 75; // Stronger incentive for lethal
        }
        else if (playerVeryLowHp && damageOutput > 0) {
            currentScore += damageOutput * 1.5;
        }

        // Consider own passives (simple check)
        for (const auto& p : botCharacter.getPassives()) {
            if (p.trigger == static_cast<PassiveTrigger>(m)) { // ON_WIN_MOVE
                currentScore += 5.0;
            }
            // Hypothetical tie. Assumes player might also pick 'm'.
            if (p.trigger == PassiveTrigger::ON_TIE) {
                currentScore += 2.0; // Small bonus if bot has a tie passive
            }
        }

        // Penalty for repeating move
        if (m == previousBotMove && previousBotMove != 0) { // Check previousBotMove != 0
            currentScore -= K_REPEATED_MOVE_PENALTY / 2.0; // Less penalty for easy AI
        }

        potentialMoves.emplace_back(m, currentScore);
    }

    std::shuffle(potentialMoves.begin(), potentialMoves.end(), gen);
    std::sort(potentialMoves.begin(), potentialMoves.end(), [](const MoveChoice& a, const MoveChoice& b) {
        return a.score > b.score;
        });

    if (!potentialMoves.empty()) {
        if (distrib_100(gen) <= 35 && potentialMoves.size() > 1) { // 35% chance to pick second best
            if (std::abs(potentialMoves[0].score - potentialMoves[1].score) < 15.0) {
                return potentialMoves[1].move;
            }
        }
        return potentialMoves[0].move;
    }
    return distrib_move(gen); // Fallback
}


// --- Hard AI ---
std::vector<AISystem::MoveChoice> AISystem::scoreMovesHard(const Character& bot, const Character& player, int previousBotMove, int previousPlayerMove) {
    std::vector<MoveChoice> all_scored_moves;

    double botHpRatio = (bot.getMaxHp() > 0) ? static_cast<double>(bot.getCurrentHp()) / bot.getMaxHp() : 0.0;
    double playerHpRatio = (player.getMaxHp() > 0) ? static_cast<double>(player.getCurrentHp()) / player.getMaxHp() : 0.0;
    bool botDesperate = botHpRatio < K_BOT_LOW_HP_THRESHOLD && botHpRatio > 0; // Don't be desperate if already dead
    bool playerVulnerable = playerHpRatio < K_PLAYER_LOW_HP_THRESHOLD && playerHpRatio > 0;
    bool botHasLead = botHpRatio > playerHpRatio + 0.25;

    for (int botMove = 1; botMove <= 3; ++botMove) {
        double totalScenarioScore = 0.0;
        std::stringstream reasoning_ss;
        reasoning_ss << "Move " << moveToString(botMove) << ": AvgScenario(";

        double sumOfIndividualScenarios = 0;
        for (int playerResponseMove = 1; playerResponseMove <= 3; ++playerResponseMove) {
            sumOfIndividualScenarios += evaluateSingleScenario(botMove, playerResponseMove, bot, player);
        }
        totalScenarioScore = sumOfIndividualScenarios / 3.0; // Average outcome
        reasoning_ss << totalScenarioScore << ") ";


        // --- Apply situational modifiers ---
        int botDamageForThisMove = getEstimatedDamage(bot, botMove);
        if (botDesperate) {
            reasoning_ss << "[Desperate] ";
            for (const auto& p : bot.getPassives()) {
                bool winsWithThisMove = (botMove == 1 && playerHpRatio < 0.5) || (botMove == 2 && playerHpRatio < 0.5) || (botMove == 3 && playerHpRatio < 0.5); // Simplistic win check
                if (((p.trigger == static_cast<PassiveTrigger>(botMove) && winsWithThisMove) || p.trigger == PassiveTrigger::ON_TIE) &&
                    (p.effect == PassiveEffect::HEAL_SELF_FLAT || p.effect == PassiveEffect::HEAL_SELF_PERCENT_CURRENT)) {
                    totalScenarioScore += K_DESPERATION_HEAL_BONUS;
                    reasoning_ss << "HealPot+ ";
                    break;
                }
            }
        }

        if (playerVulnerable) {
            reasoning_ss << "[PlayerVuln] ";
            if (player.getCurrentHp() - botDamageForThisMove <= 0 && botDamageForThisMove > 0) {
                totalScenarioScore += K_AGGRESSION_LETHAL_ATTEMPT_BONUS;
                reasoning_ss << "LethalTry+ ";
            }
        }

        if (botHasLead && !playerVulnerable) {
            totalScenarioScore *= K_HP_LEAD_CONSERVATION_FACTOR;
            reasoning_ss << "[Conserving(" << K_HP_LEAD_CONSERVATION_FACTOR << ")] ";
        }

        if (botMove == previousBotMove && previousBotMove != 0) {
            totalScenarioScore -= K_REPEATED_MOVE_PENALTY;
            reasoning_ss << "[Repeat(-" << K_REPEATED_MOVE_PENALTY << ")] ";
        }

        if (previousPlayerMove != 0 && botMove == getWinningMoveAgainst(previousPlayerMove)) {
            totalScenarioScore += 7.5;
            reasoning_ss << "[CounterPrev(+7.5)] ";
        }

        totalScenarioScore += botDamageForThisMove * K_MOVE_BASE_DAMAGE_BIAS;
        reasoning_ss << "BaseDmgBias(" << botDamageForThisMove * K_MOVE_BASE_DAMAGE_BIAS << ") ";

        all_scored_moves.emplace_back(botMove, totalScenarioScore, reasoning_ss.str());
    }
    return all_scored_moves;
}

double AISystem::evaluateSingleScenario(int botMove, int playerResponseMove, const Character& bot, const Character& player) {
    double scenarioScore = 0.0;
    int rpsWinner = 0;

    if (botMove == playerResponseMove) rpsWinner = 0;
    else if ((botMove == 1 && playerResponseMove == 3) || (botMove == 2 && playerResponseMove == 1) || (botMove == 3 && playerResponseMove == 2)) rpsWinner = 1;
    else rpsWinner = 2;

    // Pass original states for accurate % HP passive evaluation if passive triggers before main damage resolution
    Character originalBotState = bot;
    Character originalPlayerState = player;

    if (rpsWinner == 1) { // Bot wins RPS
        int damageDealtByBot = getEstimatedDamage(bot, botMove);
        scenarioScore += damageDealtByBot * K_DAMAGE_DEALT_PER_HP;
        if (player.getCurrentHp() > 0 && player.getCurrentHp() - damageDealtByBot <= 0) { // Check if player was alive
            scenarioScore += K_LETHAL_BONUS;
        }

        for (const auto& p : bot.getPassives()) { // Bot's ON_WIN_MOVE or AFTER_ANY_ATTACK
            if (p.trigger == static_cast<PassiveTrigger>(botMove) || p.trigger == PassiveTrigger::AFTER_ANY_ATTACK) {
                scenarioScore += evaluatePassiveOutcome(p, bot, player, true, originalBotState, originalPlayerState);
            }
        }
        for (const auto& p : player.getPassives()) { // Player's ON_LOSE_MOVE or AFTER_TAKING_HIT
            if (p.trigger == static_cast<PassiveTrigger>(playerResponseMove + 3) || p.trigger == PassiveTrigger::AFTER_TAKING_HIT) {
                scenarioScore += evaluatePassiveOutcome(p, player, bot, false, originalPlayerState, originalBotState);
            }
        }
    }
    else if (rpsWinner == 2) { // Player wins RPS
        int damageTakenByBot = getEstimatedDamage(player, playerResponseMove);
        scenarioScore -= damageTakenByBot * K_DAMAGE_TAKEN_PER_HP;
        if (bot.getCurrentHp() > 0 && bot.getCurrentHp() - damageTakenByBot <= 0) { // Check if bot was alive
            scenarioScore -= K_DEATH_PENALTY;
        }

        for (const auto& p : bot.getPassives()) { // Bot's ON_LOSE_MOVE or AFTER_TAKING_HIT
            if (p.trigger == static_cast<PassiveTrigger>(botMove + 3) || p.trigger == PassiveTrigger::AFTER_TAKING_HIT) {
                scenarioScore += evaluatePassiveOutcome(p, bot, player, true, originalBotState, originalPlayerState);
            }
        }
        for (const auto& p : player.getPassives()) { // Player's ON_WIN_MOVE or AFTER_ANY_ATTACK
            if (p.trigger == static_cast<PassiveTrigger>(playerResponseMove) || p.trigger == PassiveTrigger::AFTER_ANY_ATTACK) {
                scenarioScore += evaluatePassiveOutcome(p, player, bot, false, originalPlayerState, originalBotState);
            }
        }
    }
    else { // Tie
        scenarioScore += K_TIE_OUTCOME_BASE;
        for (const auto& p : bot.getPassives()) {
            if (p.trigger == PassiveTrigger::ON_TIE) {
                scenarioScore += evaluatePassiveOutcome(p, bot, player, true, originalBotState, originalPlayerState);
            }
        }
        for (const auto& p : player.getPassives()) {
            if (p.trigger == PassiveTrigger::ON_TIE) {
                scenarioScore += evaluatePassiveOutcome(p, player, bot, false, originalPlayerState, originalBotState);
            }
        }
    }
    return scenarioScore;
}

double AISystem::evaluatePassiveOutcome(const Passive& passive, const Character& self, const Character& opponent, bool selfIsActor, const Character& originalSelfState, const Character& originalOpponentState) {
    double effectStrength = 0;
    int selfCurrentHpForEval = originalSelfState.getCurrentHp(); // Use original state for % calculations
    int opponentCurrentHpForEval = originalOpponentState.getCurrentHp();

    switch (passive.effect) {
    case PassiveEffect::HEAL_SELF_FLAT:
        effectStrength = passive.value * K_PASSIVE_HEAL_MULT;
        if (selfCurrentHpForEval > 0 && selfCurrentHpForEval <= passive.value && selfIsActor)
            effectStrength += K_DEATH_PENALTY / 4.0; // Bonus for heal preventing "death" from this passive's perspective
        break;
    case PassiveEffect::DAMAGE_OPPONENT_FLAT:
        effectStrength = passive.value * K_PASSIVE_DAMAGE_MULT;
        if (opponentCurrentHpForEval > 0 && opponentCurrentHpForEval <= passive.value)
            effectStrength += K_LETHAL_BONUS / 4.0; // Bonus if passive damage is lethal
        break;
    case PassiveEffect::INCREASE_NEXT_ATTACK_FLAT:
        effectStrength = passive.value * K_PASSIVE_BUFF_MULT;
        break;
    case PassiveEffect::INCREASE_ROCK_DMG_PERM:
    case PassiveEffect::INCREASE_PAPER_DMG_PERM:
    case PassiveEffect::INCREASE_SCISSORS_DMG_PERM:
        effectStrength = passive.value * K_PASSIVE_PERM_BUFF_MULT;
        break;
    case PassiveEffect::HEAL_SELF_PERCENT_CURRENT: {
        int healAmount = (selfCurrentHpForEval * passive.value) / 100;
        effectStrength = healAmount * K_PASSIVE_HEAL_MULT;
        if (selfCurrentHpForEval > 0 && selfCurrentHpForEval <= healAmount && selfIsActor)
            effectStrength += K_DEATH_PENALTY / 4.0;
        break;
    }
    case PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT: {
        int damageAmount = (opponentCurrentHpForEval * passive.value) / 100;
        effectStrength = damageAmount * K_PASSIVE_DAMAGE_MULT;
        if (opponentCurrentHpForEval > 0 && opponentCurrentHpForEval <= damageAmount)
            effectStrength += K_LETHAL_BONUS / 4.0;
        break;
    }
    case PassiveEffect::NONE: default: break;
    }
    return selfIsActor ? effectStrength : -effectStrength;
}

int AISystem::selectMoveFromScores(const std::vector<MoveChoice>& scoredMovesInput, const Character& bot, const Character& player) {
    if (scoredMovesInput.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib_move(1, 3);
        return distrib_move(gen);
    }

    std::vector<MoveChoice> scoredMoves = scoredMovesInput;

    // Sort for debug display & finding max_score easily, highest score first
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveChoice& a, const MoveChoice& b) {
        return a.score > b.score;
        });

    // --- Debug Output ---
    // std::cout << "--- Bot: " << bot.getName() << " (" << bot.getCurrentHp() << " HP) vs Player: " << player.getName() << " (" << player.getCurrentHp() << " HP) ---" << std::endl;
    // for(const auto& mc : scoredMoves) {
    //     std::cout << "  Option: " << moveToString(mc.move) << ", Score: " << mc.score << ", Reason: " << mc.debugReasoning << std::endl;
    // }
    // --- End Debug ---

    std::vector<double> probabilities;
    double max_score = -std::numeric_limits<double>::infinity();
    if (!scoredMoves.empty()) {
        max_score = scoredMoves[0].score; // Since it's sorted
    }

    double sum_exp_scores = 0.0;

    // If K_SELECTION_TEMPERATURE is very low, act greedily
    if (K_SELECTION_TEMPERATURE < 0.01 && !scoredMoves.empty()) {
        // std::cout << "AI CHOSE (Greedy): " << moveToString(scoredMoves[0].move) << std::endl;
        return scoredMoves[0].move;
    }

    for (const auto& mc : scoredMoves) {
        // Subtract max_score before exponentiating to prevent overflow and improve numerical stability
        double exp_val = std::exp((mc.score - max_score) / K_SELECTION_TEMPERATURE);
        probabilities.push_back(exp_val);
        sum_exp_scores += exp_val;
    }

    if (sum_exp_scores == 0 || probabilities.empty()) {
        // This can happen if all scores are extremely low and result in exp(-inf) -> 0
        // Or if scoredMoves was empty initially
        // std::cout << "AI CHOSE (Fallback - Zero Sum or Empty): " << moveToString(scoredMoves.empty() ? (rand()%3+1) : scoredMoves[0].move) << std::endl;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib_move(1, 3);
        return scoredMoves.empty() ? distrib_move(gen) : scoredMoves[0].move; // Fallback to best or random
    }

    for (size_t i = 0; i < probabilities.size(); ++i) {
        probabilities[i] /= sum_exp_scores; // Normalize to get actual probabilities
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist(probabilities.begin(), probabilities.end());

    int chosenIndex = dist(gen);
    // std::cout << "AI CHOSE (Probabilistic): " << moveToString(scoredMoves[chosenIndex].move) << " with prob " << probabilities[chosenIndex] << std::endl;
    return scoredMoves[chosenIndex].move;
}


// --- Utility Implementations ---
int AISystem::getWinningMoveAgainst(int opponentMove) {
    if (opponentMove == 1) return 2; // Paper beats Rock
    if (opponentMove == 2) return 3; // Scissors beats Paper
    if (opponentMove == 3) return 1; // Rock beats Scissors
    // Should not be called with opponentMove == 0, but handle defensively
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_move(1, 3);
    return distrib_move(gen);
}

int AISystem::getLosingMoveAgainst(int opponentMove) {
    if (opponentMove == 1) return 3; // Scissors loses to Rock
    if (opponentMove == 2) return 1; // Rock loses to Paper
    if (opponentMove == 3) return 2; // Paper loses to Scissors
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib_move(1, 3);
    return distrib_move(gen);
}

int AISystem::getTyingMove(int ownMove) { // More accurately, what move would tie if opponent played it
    return ownMove;
}