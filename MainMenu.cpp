#include "MainMenu.h"
#include "CharacterManager.h" 
#include "Utils.h"           
#include <iostream>
#include <cstdlib> 
#include <ctime>   
#include "GauntletGame.h"

using namespace std;

MainMenu::MainMenu() : exitGame(false) {
    srand(static_cast<unsigned int>(time(nullptr)));
    loadCharacters(); // Load characters at the start
}

void MainMenu::displayMenu() {
    system("cls");
    cout << "==================================\n";
    cout << "=           PIC BATTLE           =\n";
    cout << "==================================\n\n";
    cout << "1. Start Battle\n";
    cout << "2. Debug Mode Battle\n";
    cout << "3. Gauntlet Mode\n"; // New Option
    cout << "4. Character Creator\n";
    cout << "5. Exit\n\n";        // Adjust numbering
    cout << "Enter your choice: ";
}

void MainMenu::displayCreatorMenu() {
    system("cls");
    cout << "==================================\n";
    cout << "=       CHARACTER CREATOR        =\n";
    cout << "==================================\n\n";
    cout << "1. Create New Character\n";
    cout << "2. View All Characters\n";
    cout << "3. Delete Custom Character\n";
    cout << "4. Back to Main Menu\n\n";
    cout << "Enter your choice: ";
}

void MainMenu::runCreator() {
    bool back = false;
    while (!back) {
        displayCreatorMenu();
        int choice = getIntInput("", 1, 4);
        switch (choice) {
        case 1: createNewCharacter(); break;
        case 2: viewCharacters(); break;
        case 3: deleteCharacter(); break;
        case 4: back = true; break;
        }
    }
}

void MainMenu::run() {
    while (!exitGame) {
        displayMenu();
        int choice = getIntInput("", 1, 5); 
        switch (choice) {
        case 1:
            game.setDebugMode(false);
            game.play();
            break;
        case 2:
            game.setDebugMode(true);
            game.play();
            break;
        case 3: 
            gauntletGame.play();
            break;
        case 4: 
            runCreator();
            break;
        case 5: 
            exitGame = true;
            cout << "\nSaving characters and exiting...\n";
            saveCharacters(); // Ensure custom characters are saved
            // Gauntlet unlocks are saved by GauntletGame itself.
            cout << "GAME OVER!\n";
            break;
        }
    }
}