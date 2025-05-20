#ifndef CHARACTER_H
#define CHARACTER_H

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "PassiveSystem.h"

class Character {
protected:
    std::string name;
    int maxHp;
    int currentHp;

    // Current, potentially buffed, damage values used in battle
    int baseRockDamage;
    int basePaperDamage;
    int baseScissorsDamage;

    // Store original base stats for reset ---
    int originalRockDamage;
    int originalPaperDamage;
    int originalScissorsDamage;
   

    int bonusDamageNextAttack;
    std::vector<Passive> passives;
    std::string characterType; // "BUILTIN" or "CUSTOM"

public:
    // Constructors need to initialize original stats as well
    Character(const std::string& n, int hp, int rock, int paper, int scissors, std::string type = "BUILTIN");
    Character(const std::string& n, int hp, int rock, int paper, int scissors, std::vector<Passive> p, std::string type = "CUSTOM");

    void resetStatsForNewBattle(); 
    virtual ~Character();

    std::string getName() const;
    int getMaxHp() const;
    int getCurrentHp() const;
    int getRockDamage() const;    // Returns current baseRockDamage
    int getPaperDamage() const;   // Returns current basePaperDamage
    int getScissorsDamage() const;// Returns current baseScissorsDamage
    int getBonusDamageNextAttack() const;
    const std::vector<Passive>& getPassives() const;
    std::string getType() const;

    // Getters for original stats (optional, mostly for debugging or if needed elsewhere) ---
    // int getOriginalRockDamage() const;
    // int getOriginalPaperDamage() const;
    // int getOriginalScissorsDamage() const;
    // --- END NEW ---


    bool isDefeated() const;

    virtual void takeDamage(int damage);
    virtual void heal(int amount);
    virtual int calculateDamage(int move);
    virtual void checkAndApplyPassives(PassiveTrigger triggerType, Character& self, Character& opponent, int move = 0, bool didWin = false);
    void addBonusDamageNextAttack(int amount);

    // These now modify the current battle stats
    void increaseBaseRockDamage(int amount);
    void increaseBasePaperDamage(int amount);
    void increaseBaseScissorsDamage(int amount);

    virtual std::string getMoveDescription(int move) const;
    virtual std::string getShortDescription() const;
    virtual std::string getFullDescription() const;

    void resetTurnState();
};


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