#include "PassiveSystem.h"
#include <string>
#include <iostream> 

using std::string;
using std::to_string;


std::string Passive::getDescription() const {
    string triggerDesc = "Unknown Trigger";
    switch (trigger) {
    case PassiveTrigger::NONE: triggerDesc = "No trigger"; break;
    case PassiveTrigger::ON_WIN_ROCK: triggerDesc = "On winning with Rock"; break;
    case PassiveTrigger::ON_WIN_PAPER: triggerDesc = "On winning with Paper"; break;
    case PassiveTrigger::ON_WIN_SCISSORS: triggerDesc = "On winning with Scissors"; break;
    case PassiveTrigger::ON_LOSE_ROCK: triggerDesc = "On losing to Rock"; break;
    case PassiveTrigger::ON_LOSE_PAPER: triggerDesc = "On losing to Paper"; break;
    case PassiveTrigger::ON_LOSE_SCISSORS: triggerDesc = "On losing to Scissors"; break;
    case PassiveTrigger::ON_TIE: triggerDesc = "On a tie"; break;
    case PassiveTrigger::ON_HP_BELOW_PERCENT: triggerDesc = "When HP is below " + to_string(threshold) + "%"; break;
    case PassiveTrigger::ON_TURN_START: triggerDesc = "At the start of your turn"; break;
    case PassiveTrigger::AFTER_ANY_ATTACK: triggerDesc = "After you attack"; break;
    case PassiveTrigger::AFTER_TAKING_HIT: triggerDesc = "After taking damage"; break;
    }

    string effectDesc = "Unknown Effect";
    switch (effect) {
    case PassiveEffect::NONE: effectDesc = "no effect"; break;
    case PassiveEffect::HEAL_SELF_FLAT: effectDesc = "heal self for " + to_string(value) + " HP"; break;
    case PassiveEffect::DAMAGE_OPPONENT_FLAT: effectDesc = "deal " + to_string(value) + " damage to opponent"; break;
    case PassiveEffect::INCREASE_NEXT_ATTACK_FLAT: effectDesc = "increase next attack by " + to_string(value) + " damage"; break;
    case PassiveEffect::INCREASE_ROCK_DMG_PERM: effectDesc = "permanently increase Rock damage by " + to_string(value); break;
    case PassiveEffect::INCREASE_PAPER_DMG_PERM: effectDesc = "permanently increase Paper damage by " + to_string(value); break;
    case PassiveEffect::INCREASE_SCISSORS_DMG_PERM: effectDesc = "permanently increase Scissors damage by " + to_string(value); break;
    case PassiveEffect::HEAL_SELF_PERCENT_CURRENT: effectDesc = "heal self for " + to_string(value) + "% of current HP"; break;
    case PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT: effectDesc = "deal " + to_string(value) + "% of opponent's current HP as damage"; break;
    }

    return triggerDesc + ": " + effectDesc + ".";
}

std::string Passive::toString() const {
    std::stringstream ss;
    ss << static_cast<int>(trigger) << ","
        << static_cast<int>(effect) << ","
        << value << ","
        << threshold;
    return ss.str();
}

Passive Passive::fromString(const std::string& s) {
    Passive p;
    std::stringstream ss(s);
    std::string segment;
    int data[4] = { 0 };
    int i = 0;

    while (std::getline(ss, segment, ',') && i < 4) {
        try {
            data[i++] = std::stoi(segment);
        }
        catch (...) { /* Handle parsing error if needed */ }
    }

    if (i >= 2) { // Need at least trigger and effect
        p.trigger = static_cast<PassiveTrigger>(data[0]);
        p.effect = static_cast<PassiveEffect>(data[1]);
        p.value = data[2];
        p.threshold = data[3];
    }
    return p;
}