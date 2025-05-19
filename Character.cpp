#include "Character.h"
#include "PassiveSystem.h" 
#include <iostream>     
#include <algorithm>    
#include <sstream>      

using namespace std;

// Constructor for built-in characters (or custom without initial passives)
Character::Character(const string& n, int hp, int rock, int paper, int scissors, string type)
    : name(n), maxHp(hp), currentHp(hp), baseRockDamage(rock), basePaperDamage(paper),
    baseScissorsDamage(scissors), bonusDamageNextAttack(0), characterType(type) {
}

// Constructor for custom characters (includes passives) or built-ins with passives
Character::Character(const string& n, int hp, int rock, int paper, int scissors, vector<Passive> p, string type)
    : name(n), maxHp(hp), currentHp(hp), baseRockDamage(rock), basePaperDamage(paper),
    baseScissorsDamage(scissors), bonusDamageNextAttack(0), passives(std::move(p)), characterType(type) {
}

Character::~Character() {}

// --- Getters ---
string Character::getName() const { return name; }
int Character::getMaxHp() const { return maxHp; }
int Character::getCurrentHp() const { return currentHp; }
int Character::getRockDamage() const { return baseRockDamage; }
int Character::getPaperDamage() const { return basePaperDamage; }
int Character::getScissorsDamage() const { return baseScissorsDamage; }
const vector<Passive>& Character::getPassives() const { return passives; }
string Character::getType() const { return characterType; }


// --- Status Checks ---
bool Character::isDefeated() const { return currentHp <= 0; }

// --- Actions ---
void Character::takeDamage(int damage) {
    currentHp -= damage;
    if (currentHp < 0) currentHp = 0;
}

void Character::heal(int amount) {
    currentHp += amount;
    if (currentHp > maxHp) currentHp = maxHp;
}

int Character::calculateDamage(int move) {
    int damage = 0;
    switch (move) {
    case 1: damage = baseRockDamage; break;
    case 2: damage = basePaperDamage; break;
    case 3: damage = baseScissorsDamage; break;
    }
    damage += bonusDamageNextAttack;
    bonusDamageNextAttack = 0; // Reset bonus damage after calculating
    return damage;
}

void Character::addBonusDamageNextAttack(int amount) {
    bonusDamageNextAttack += amount;
}

void Character::increaseBaseRockDamage(int amount) { baseRockDamage += amount; }
void Character::increaseBasePaperDamage(int amount) { basePaperDamage += amount; }
void Character::increaseBaseScissorsDamage(int amount) { baseScissorsDamage += amount; }

void Character::resetTurnState() {
    for (auto& p : passives) {
        p.triggeredThisTurn = false;
    }
}

string Character::getMoveDescription(int move) const {
    string desc;
    int current_bonus = bonusDamageNextAttack; // Show potential bonus
    switch (move) {
    case 1: desc = "Rock (" + to_string(baseRockDamage + current_bonus) + " dmg)"; break;
    case 2: desc = "Paper (" + to_string(basePaperDamage + current_bonus) + " dmg)"; break;
    case 3: desc = "Scissors (" + to_string(baseScissorsDamage + current_bonus) + " dmg)"; break;
    default: desc = "Unknown"; break;
    }
    return desc;
}

string Character::getShortDescription() const {
    return name + " (" + to_string(maxHp) + " HP, R:" + to_string(baseRockDamage)
        + " P:" + to_string(basePaperDamage) + " S:" + to_string(baseScissorsDamage) + ")";
}

string Character::getFullDescription() const {
    stringstream ss;
    ss << getShortDescription();
    if (!passives.empty()) {
        ss << "\n  Passives:";
        for (const auto& p : passives) {
            ss << "\n    - " << p.getDescription();
        }
    }
    return ss.str();
}

void Character::checkAndApplyPassives(PassiveTrigger triggerType, Character& self, Character& opponent, int move, bool didWin) {
    // Check HP threshold passives separately if HP changed
    if (triggerType == PassiveTrigger::ON_HP_BELOW_PERCENT) {
        for (auto& p : passives) {
            if (!p.triggeredThisTurn && p.trigger == PassiveTrigger::ON_HP_BELOW_PERCENT) {
                int hpPercent = (maxHp > 0) ? (static_cast<double>(currentHp) / maxHp * 100) : 0;
                if (hpPercent <= p.threshold) {
                    cout << self.getName() << "'s passive triggered (" << p.getDescription() << ")!\n";
                    p.triggeredThisTurn = true;

                    switch (p.effect) {
                    case PassiveEffect::HEAL_SELF_FLAT: self.heal(p.value); cout << "  Healed " << p.value << " HP.\n"; break;
                    case PassiveEffect::DAMAGE_OPPONENT_FLAT: opponent.takeDamage(p.value); cout << "  Dealt " << p.value << " damage to " << opponent.getName() << ".\n"; break;
                    case PassiveEffect::INCREASE_NEXT_ATTACK_FLAT: self.addBonusDamageNextAttack(p.value); cout << "  Next attack + " << p.value << " damage.\n"; break;
                    case PassiveEffect::INCREASE_ROCK_DMG_PERM: self.increaseBaseRockDamage(p.value); cout << "  Rock damage permanently increased by " << p.value << ".\n"; break;
                    case PassiveEffect::INCREASE_PAPER_DMG_PERM: self.increaseBasePaperDamage(p.value); cout << "  Paper damage permanently increased by " << p.value << ".\n"; break;
                    case PassiveEffect::INCREASE_SCISSORS_DMG_PERM: self.increaseBaseScissorsDamage(p.value); cout << "  Scissors damage permanently increased by " << p.value << ".\n"; break;
                    case PassiveEffect::HEAL_SELF_PERCENT_CURRENT: {
                        int healAmount = (self.getCurrentHp() * p.value) / 100;
                        self.heal(healAmount);
                        cout << "  Healed " << healAmount << " HP (" << p.value << "% of current HP).\n";
                        break;
                    }
                    case PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT: {
                        int damageAmount = (opponent.getCurrentHp() * p.value) / 100;
                        opponent.takeDamage(damageAmount);
                        cout << "  Dealt " << damageAmount << " damage to " << opponent.getName() << " (" << p.value << "% of their current HP).\n";
                        break;
                    }
                    default: break;
                    }
                }
            }
        }
        return; // HP check is separate
    }

    // Check other trigger types
    for (auto& p : passives) {
        if (p.triggeredThisTurn) continue;

        bool triggerMatches = false;
        switch (triggerType) {
        case PassiveTrigger::ON_WIN_ROCK: triggerMatches = (p.trigger == triggerType && move == 1 && didWin); break;
        case PassiveTrigger::ON_WIN_PAPER: triggerMatches = (p.trigger == triggerType && move == 2 && didWin); break;
        case PassiveTrigger::ON_WIN_SCISSORS: triggerMatches = (p.trigger == triggerType && move == 3 && didWin); break;
        case PassiveTrigger::ON_LOSE_ROCK: triggerMatches = (p.trigger == triggerType && move == 1 && !didWin); break;
        case PassiveTrigger::ON_LOSE_PAPER: triggerMatches = (p.trigger == triggerType && move == 2 && !didWin); break;
        case PassiveTrigger::ON_LOSE_SCISSORS: triggerMatches = (p.trigger == triggerType && move == 3 && !didWin); break;
        case PassiveTrigger::ON_TIE: triggerMatches = (p.trigger == triggerType); break;
        case PassiveTrigger::ON_TURN_START: triggerMatches = (p.trigger == triggerType); break;
        case PassiveTrigger::AFTER_ANY_ATTACK: triggerMatches = (p.trigger == triggerType); break;
        case PassiveTrigger::AFTER_TAKING_HIT: triggerMatches = (p.trigger == triggerType); break;
        default: break;
        }

        if (triggerMatches) {
            cout << self.getName() << "'s passive triggered (" << p.getDescription() << ")!\n";
            p.triggeredThisTurn = true;

            switch (p.effect) {
            case PassiveEffect::HEAL_SELF_FLAT: self.heal(p.value); cout << "  Healed " << p.value << " HP.\n"; break;
            case PassiveEffect::DAMAGE_OPPONENT_FLAT: opponent.takeDamage(p.value); cout << "  Dealt " << p.value << " damage to " << opponent.getName() << ".\n"; break;
            case PassiveEffect::INCREASE_NEXT_ATTACK_FLAT: self.addBonusDamageNextAttack(p.value); cout << "  Next attack + " << p.value << " damage.\n"; break;
            case PassiveEffect::INCREASE_ROCK_DMG_PERM: self.increaseBaseRockDamage(p.value); cout << "  Rock damage permanently increased by " << p.value << ".\n"; break;
            case PassiveEffect::INCREASE_PAPER_DMG_PERM: self.increaseBasePaperDamage(p.value); cout << "  Paper damage permanently increased by " << p.value << ".\n"; break;
            case PassiveEffect::INCREASE_SCISSORS_DMG_PERM: self.increaseBaseScissorsDamage(p.value); cout << "  Scissors damage permanently increased by " << p.value << ".\n"; break;
            case PassiveEffect::HEAL_SELF_PERCENT_CURRENT: {
                int healAmount = (self.getCurrentHp() * p.value) / 100;
                self.heal(healAmount);
                cout << "  Healed " << healAmount << " HP (" << p.value << "% of current HP).\n";
                break;
            }
            case PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT: {
                int damageAmount = (opponent.getCurrentHp() * p.value) / 100;
                opponent.takeDamage(damageAmount);
                cout << "  Dealt " << damageAmount << " damage to " << opponent.getName() << " (" << p.value << "% of their current HP).\n";
                break;
            }
            default: break;
            }
            if (opponent.isDefeated()) {
                cout << opponent.getName() << " was defeated by the passive effect!\n";
            }
            if (self.isDefeated()) {
                cout << self.getName() << " was defeated by their own passive effect!?\n";
            }
        }
    }
}


// --- Built-in Character Definitions ---
OG::OG() : Character("OG", 20, 1, 2, 3, {}, "BUILTIN") {}

Helios::Helios() : Character("Helios", 25, 1, 0, 2,
    {
        Passive(PassiveTrigger::ON_WIN_PAPER, PassiveEffect::HEAL_SELF_FLAT, 5)
    }, "BUILTIN") {
}

Duran::Duran() : Character("Duran", 15, 2, 1, 3,
    {
        Passive(PassiveTrigger::ON_WIN_SCISSORS, PassiveEffect::INCREASE_NEXT_ATTACK_FLAT, 3)
    }, "BUILTIN") {
}

Philip::Philip() : Character("Philip", 18, 1, 2, 1,
    {
        Passive(PassiveTrigger::ON_TIE, PassiveEffect::DAMAGE_OPPONENT_FLAT, 1)
    }, "BUILTIN") {
}

Razor::Razor() : Character("Razor", 7, 3, 4, 5, {}, "BUILTIN") {}

Sunny::Sunny() : Character("Sunny", 14, 1, 3, 2,
    {
        Passive(PassiveTrigger::ON_HP_BELOW_PERCENT, PassiveEffect::INCREASE_ROCK_DMG_PERM, 4, 28),
        Passive(PassiveTrigger::ON_HP_BELOW_PERCENT, PassiveEffect::INCREASE_PAPER_DMG_PERM, 2, 28),
        Passive(PassiveTrigger::ON_HP_BELOW_PERCENT, PassiveEffect::INCREASE_SCISSORS_DMG_PERM, 3, 28)
    }, "BUILTIN") {
}