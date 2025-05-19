#ifndef MAINMENU_H
#define MAINMENU_H

#include "Game.h" // For Game object member
// CharacterManager functions like loadCharacters, saveCharacters are called directly
#include "CharacterManager.h" 
#include "GauntletGame.h"
#include <cstdlib> 
#include <ctime>   

using namespace std;

class MainMenu {
private:
    Game game;
    GauntletGame gauntletGame;
    bool exitGame;

    void displayMenu();
    void displayCreatorMenu();
    void runCreator();

public:
    MainMenu();
    void run();
};

#endif // MAINMENU_H
