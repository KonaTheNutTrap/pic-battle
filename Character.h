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
    int baseRockDamage;
    int basePaperDamage;
    int baseScissorsDamage;
    int bonusDamageNextAttack;
    std::vector<Passive> passives;
    std::string characterType;

public:
    Character(const std::string& n, int hp, int rock, int paper, int scissors, std::string type = "BUILTIN");
    Character(const std::string& n, int hp, int rock, int paper, int scissors, std::vector<Passive> p, std::string type = "CUSTOM");

    void resetStatsForNewBattle();
    virtual ~Character();

    std::string getName() const;
    int getMaxHp() const;
    int getCurrentHp() const;
    int getRockDamage() const;
    int getPaperDamage() const;
    int getScissorsDamage() const;
    int getBonusDamageNextAttack() const; // Added for AI
    const std::vector<Passive>& getPassives() const;
    std::string getType() const;

    bool isDefeated() const;

    virtual void takeDamage(int damage);
    virtual void heal(int amount);
    virtual int calculateDamage(int move);
    virtual void checkAndApplyPassives(PassiveTrigger triggerType, Character& self, Character& opponent, int move = 0, bool didWin = false);
    void addBonusDamageNextAttack(int amount);
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