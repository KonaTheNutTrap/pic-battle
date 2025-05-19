#ifndef PASSIVESYSTEM_H
#define PASSIVESYSTEM_H

#include <string>
#include <sstream> 
#include <vector>  


using namespace std;

// Define triggers for passives
enum class PassiveTrigger {
    NONE,
    ON_WIN_ROCK,
    ON_WIN_PAPER,
    ON_WIN_SCISSORS,
    ON_LOSE_ROCK,
    ON_LOSE_PAPER,
    ON_LOSE_SCISSORS,
    ON_TIE,
    ON_HP_BELOW_PERCENT,
    ON_TURN_START,
    AFTER_ANY_ATTACK,
    AFTER_TAKING_HIT
};

// Define effects of passives
enum class PassiveEffect {
    NONE,
    HEAL_SELF_FLAT, // Heal self
    DAMAGE_OPPONENT_FLAT, // Damage opponent 
    INCREASE_NEXT_ATTACK_FLAT, // Bonus damage for the next attack only
    INCREASE_ROCK_DMG_PERM,   // Permanent increase
    INCREASE_PAPER_DMG_PERM,  // Permanent increase
    INCREASE_SCISSORS_DMG_PERM,// Permanent increase
    HEAL_SELF_PERCENT_CURRENT, // Heal based on current HP
    DAMAGE_OPPONENT_PERCENT_CURRENT // Damage opponent based on their current HP
};

// Structure to hold passive details
struct Passive {
    PassiveTrigger trigger = PassiveTrigger::NONE;
    PassiveEffect effect = PassiveEffect::NONE;
    int value = 0;       // Amount for heal/damage/boost OR percentage for HP trigger
    int threshold = 0;   // e.g., HP percentage for ON_HP_BELOW_PERCENT
    bool triggeredThisTurn = false; // To prevent multiple triggers per turn if needed

    // Default constructor
    Passive() = default;

    // Parameterized constructor
    Passive(PassiveTrigger t, PassiveEffect e, int v, int th = 0)
        : trigger(t), effect(e), value(v), threshold(th), triggeredThisTurn(false) {
    }

    // Function to get string representation for saving/display
    string toString() const {
        stringstream ss;
        ss << static_cast<int>(trigger) << ","
            << static_cast<int>(effect) << ","
            << value << ","
            << threshold;
        return ss.str();
    }

    // Function to create Passive from string representation
    static Passive fromString(const string& s) {
        Passive p;
        stringstream ss(s);
        string segment;
        int data[4] = { 0 };
        int i = 0;

        while (getline(ss, segment, ',') && i < 4) {
            try {
                data[i++] = stoi(segment);
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

    // Function to get a user-friendly description
    string getDescription() const;
};

#endif // PASSIVESYSTEM_H