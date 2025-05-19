#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

#include "Character.h"
#include "PassiveSystem.h"
#include <string>
#include <vector> // For std::vector
#include <memory> // For std::unique_ptr

// Global available characters list
extern std::vector<std::unique_ptr<Character>> availableCharacters;

// Save file constant
extern const std::string SAVE_FILE;

// Function declarations
void loadCharacters();
void saveCharacters();
void displayPassiveOptions();
void createNewCharacter();
void viewCharacters();
void deleteCharacter();

#endif // CHARACTERMANAGER_H