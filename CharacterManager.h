#ifndef CHARACTERMANAGER_H
#define CHARACTERMANAGER_H

#include "Character.h"    
#include "PassiveSystem.h"
#include <string>
#include <memory>         

using namespace std;

// Global available characters list
extern vector<unique_ptr<Character>> availableCharacters;

// Save file constant
extern const string SAVE_FILE;

// Function declarations
void loadCharacters();
void saveCharacters();
void displayPassiveOptions(); 
void createNewCharacter();
void viewCharacters();
void deleteCharacter();

#endif // CHARACTERMANAGER_H