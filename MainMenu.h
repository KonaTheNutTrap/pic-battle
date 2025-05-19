#ifndef MAINMENU_H
#define MAINMENU_H

#include "Game.h"
#include "CharacterManager.h"
#include "GauntletGame.h"
#include "AISystem.h" 
#include <cstdlib> 
#include <ctime>   

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