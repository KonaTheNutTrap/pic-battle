#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

#include "Character.h"
#include "PassiveSystem.h"
#include <string>
#include <vector>
#include <memory>

extern std::vector<std::unique_ptr<Character>> availableCharacters;
extern const std::string SAVE_FILE;

void loadCharacters();
void saveCharacters();
void displayPassiveOptions();
void createNewCharacter();
void viewCharacters();
void editCharacter();
void deleteCharacter();

#endif // CHARACTERMANAGER_H