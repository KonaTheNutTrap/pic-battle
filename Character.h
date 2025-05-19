#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include <memory>  
#include <sstream>  
#include "PassiveSystem.h" 

using namespace std;

class Character {
protected:
    string name;
    int maxHp;
    int currentHp;
    int baseRockDamage;
    int basePaperDamage;
    int baseScissorsDamage;
    int bonusDamageNextAttack;
    vector<Passive> passives;
    string characterType; // "BUILTIN" or "CUSTOM"

public:
    // Constructor for built-in characters (can also be used for custom without passives initially)
    Character(const string& n, int hp, int rock, int paper, int scissors, string type = "BUILTIN");

    // Constructor for custom characters (includes passives) or built-ins with passives
    Character(const string& n, int hp, int rock, int paper, int scissors, vector<Passive> p, string type = "CUSTOM");

    // Resets HP to max and other temporary battle stats
    void resetStatsForNewBattle(); 

    virtual ~Character();

    // --- Getters ---
    string getName() const;
    int getMaxHp() const;
    int getCurrentHp() const;
    int getRockDamage() const;
    int getPaperDamage() const;
    int getScissorsDamage() const;
    const vector<Passive>& getPassives() const;
    string getType() const;

    // --- Status Checks ---
    bool isDefeated() const;

    // --- Actions ---
    virtual void takeDamage(int damage);
    virtual void heal(int amount);
    virtual int calculateDamage(int move);
    virtual void checkAndApplyPassives(PassiveTrigger triggerType, Character& self, Character& opponent, int move = 0, bool didWin = false);
    void addBonusDamageNextAttack(int amount);
    void increaseBaseRockDamage(int amount);
    void increaseBasePaperDamage(int amount);
    void increaseBaseScissorsDamage(int amount);

    // --- Display ---
    virtual string getMoveDescription(int move) const;
    virtual string getShortDescription() const;
    virtual string getFullDescription() const;

    // Reset state at the start of a turn
    void resetTurnState();
};

// --- Built-in Character Definitions ---
// (Declarations here, definitions in Character.cpp)
class OG : public Character {
public:
    OG();
};

class Helios : public Character {
public:
    Helios();
};

class Duran : public Character {
public:
    Duran();
};

class Philip : public Character {
public:
    Philip();
};

class Razor : public Character {
public:
    Razor();
};

class Sunny : public Character {
public:
    Sunny();
};

#endif // CHARACTER_H
