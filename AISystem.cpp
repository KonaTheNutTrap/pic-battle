#include "AISystem.h"
#include <vector>
#include <algorithm>
#include <random>
#include <iostream> 
#include <map>    

namespace {
    const double K_DAMAGE_DEALT_PER_HP = 1.0;
    const double K_LETHAL_BONUS = 100.0;
    const double K_DAMAGE_TAKEN_PER_HP = 1.2;
    const double K_DEATH_PENALTY = 120.0;
    const double K_TIE_OUTCOME_BASE = 0.0;
    const double K_MOVE_BASE_DAMAGE_BIAS = 0.1;

    const double K_PASSIVE_HEAL_MULT = 1.0;
    const double K_PASSIVE_DAMAGE_MULT = 1.1;
    const double K_PASSIVE_BUFF_MULT = 0.8;
    const double K_PASSIVE_PERM_BUFF_MULT = 1.5;

    int getEstimatedDamage(const Character& c, int move) {
        int baseDmg = 0;
        switch (move) {
        case 1: baseDmg = c.getRockDamage(); break;
        case 2: baseDmg = c.getPaperDamage(); break;
        case 3: baseDmg = c.getScissorsDamage(); break;
        }
        return baseDmg + c.getBonusDamageNextAttack();
    }
}

int AISystem::chooseMove(const Character& botCharacter, const Character& playerCharacter, AIDifficulty difficulty) {
    if (difficulty == AIDifficulty::EASY) {
        return chooseMoveEasy(botCharacter, playerCharacter);
    }
    else {
        std::vector<MoveChoice> scoredMoves;
        for (int move = 1; move <= 3; ++move) {
            scoredMoves.emplace_back(move, scoreMoveHard(move, botCharacter, playerCharacter));
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(scoredMoves.begin(), scoredMoves.end(), g);

        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveChoice& a, const MoveChoice& b) {
            return a.score > b.score;
            });

        // Debug:
        // for(const auto& mc : scoredMoves) {
        //     std::cout << "Bot Move " << mc.move << " (";
        //     if(mc.move == 1) std::cout << "R"; else if(mc.move == 2) std::cout << "P"; else std::cout << "S";
        //     std::cout << ") Score: " << mc.score << std::endl;
        // }

        if (!scoredMoves.empty()) {
            return scoredMoves[0].move;
        }
        return (rand() % 3) + 1;
    }
}

int AISystem::chooseMoveEasy(const Character& botCharacter, const Character& playerCharacter) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 100);
    std::uniform_int_distribution<> move_distrib(1, 3);


    if (distrib(gen) <= 33) {
        return move_distrib(gen);
    }

    std::vector<MoveChoice> potentialMoves;
    bool playerVeryLowHp = (static_cast<double>(playerCharacter.getCurrentHp()) / playerCharacter.getMaxHp()) < 0.20;

    for (int m = 1; m <= 3; ++m) {
        double currentScore = 0;
        int damageOutput = getEstimatedDamage(botCharacter, m);

        currentScore += damageOutput * 0.5;

        if (playerVeryLowHp && damageOutput > 0 && (playerCharacter.getCurrentHp() - damageOutput <= 0)) {
            currentScore += 50;
        }
        else if (playerVeryLowHp && damageOutput > 0) {
            currentScore += damageOutput;
        }

        for (const auto& p : botCharacter.getPassives()) {
            if (p.trigger == static_cast<PassiveTrigger>(m)) {
                currentScore += 3.0;
            }
        }
        potentialMoves.emplace_back(m, currentScore);
    }

    std::shuffle(potentialMoves.begin(), potentialMoves.end(), gen);
    std::sort(potentialMoves.begin(), potentialMoves.end(), [](const MoveChoice& a, const MoveChoice& b) {
        return a.score > b.score;
        });

    if (!potentialMoves.empty()) {
        if (distrib(gen) <= 25 && potentialMoves.size() > 1) {
            if (potentialMoves[0].score - potentialMoves[1].score < 10.0) {
                return potentialMoves[1].move;
            }
        }
        return potentialMoves[0].move;
    }
    return move_distrib(gen);
}

double AISystem::scoreMoveHard(int botMove, const Character& bot, const Character& player) {
    double totalScenarioScore = 0.0;

    for (int playerMove = 1; playerMove <= 3; ++playerMove) {
        double scenarioScoreContribution = 0.0;
        int rpsWinner = 0;

        if (botMove == playerMove) rpsWinner = 0;
        else if ((botMove == 1 && playerMove == 3) || (botMove == 2 && playerMove == 1) || (botMove == 3 && playerMove == 2)) rpsWinner = 1;
        else rpsWinner = 2;

        if (rpsWinner == 1) {
            int damageDealtByBot = getEstimatedDamage(bot, botMove);
            scenarioScoreContribution += damageDealtByBot * K_DAMAGE_DEALT_PER_HP;
            if (player.getCurrentHp() - damageDealtByBot <= 0) {
                scenarioScoreContribution += K_LETHAL_BONUS;
            }

            for (const auto& p : bot.getPassives()) {
                if (p.trigger == static_cast<PassiveTrigger>(botMove) || p.trigger == PassiveTrigger::AFTER_ANY_ATTACK) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, bot, player, true);
                }
            }
            for (const auto& p : player.getPassives()) {
                if (p.trigger == static_cast<PassiveTrigger>(playerMove + 3)) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, player, bot, false);
                }
            }
        }
        else if (rpsWinner == 2) {
            int damageTakenByBot = getEstimatedDamage(player, playerMove);
            scenarioScoreContribution -= damageTakenByBot * K_DAMAGE_TAKEN_PER_HP;
            if (bot.getCurrentHp() - damageTakenByBot <= 0) {
                scenarioScoreContribution -= K_DEATH_PENALTY;
            }

            for (const auto& p : bot.getPassives()) {
                if (p.trigger == static_cast<PassiveTrigger>(botMove + 3) || p.trigger == PassiveTrigger::AFTER_TAKING_HIT) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, bot, player, true);
                }
            }
            for (const auto& p : player.getPassives()) {
                if (p.trigger == static_cast<PassiveTrigger>(playerMove) || p.trigger == PassiveTrigger::AFTER_ANY_ATTACK) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, player, bot, false);
                }
            }
        }
        else {
            scenarioScoreContribution += K_TIE_OUTCOME_BASE;
            for (const auto& p : bot.getPassives()) {
                if (p.trigger == PassiveTrigger::ON_TIE) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, bot, player, true);
                }
            }
            for (const auto& p : player.getPassives()) {
                if (p.trigger == PassiveTrigger::ON_TIE) {
                    scenarioScoreContribution += evaluatePassiveOutcome(p, player, bot, false);
                }
            }
        }
        totalScenarioScore += scenarioScoreContribution;
    }

    int botBaseDamageForThisMove = 0;
    switch (botMove) {
    case 1: botBaseDamageForThisMove = bot.getRockDamage(); break;
    case 2: botBaseDamageForThisMove = bot.getPaperDamage(); break;
    case 3: botBaseDamageForThisMove = bot.getScissorsDamage(); break;
    }
    totalScenarioScore += botBaseDamageForThisMove * K_MOVE_BASE_DAMAGE_BIAS;

    return totalScenarioScore;
}

double AISystem::evaluatePassiveOutcome(const Passive& passive, const Character& self, const Character& opponent, bool selfIsActor) {
    double effectStrength = 0;

    switch (passive.effect) {
    case PassiveEffect::HEAL_SELF_FLAT:
        effectStrength = passive.value * K_PASSIVE_HEAL_MULT;
        break;
    case PassiveEffect::DAMAGE_OPPONENT_FLAT:
        effectStrength = passive.value * K_PASSIVE_DAMAGE_MULT;
        if (opponent.getCurrentHp() - passive.value <= 0) effectStrength += K_LETHAL_BONUS / 5.0; 
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
        int healAmount = (self.getCurrentHp() * passive.value) / 100;
        effectStrength = healAmount * K_PASSIVE_HEAL_MULT;
        break;
    }
    case PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT: {
        int damageAmount = (opponent.getCurrentHp() * passive.value) / 100;
        effectStrength = damageAmount * K_PASSIVE_DAMAGE_MULT;
        if (opponent.getCurrentHp() - damageAmount <= 0) effectStrength += K_LETHAL_BONUS / 5.0;
        break;
    }
    case PassiveEffect::NONE:
    default:
        break;
    }
    return selfIsActor ? effectStrength : -effectStrength;
}