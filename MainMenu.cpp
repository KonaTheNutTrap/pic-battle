#include "MainMenu.h"
#include "CharacterManager.h" 
#include "Utils.h"           
#include "GauntletGame.h"
#include "AISystem.h"     
#include <iostream>
#include <cstdlib> 
#include <ctime>   

using namespace std;

MainMenu::MainMenu() : exitGame(false) {
    srand(static_cast<unsigned int>(time(nullptr)));
    loadCharacters();
}

void MainMenu::displayMenu() {
    system("cls");
    cout << "=================================\n";
    cout << "=  PRECARIOUS INTUITION COMBAT  =\n";
    cout << "=================================\n\n";
    cout << "1. Start Battle (AI: "
        << (game.getAIDifficulty() == AIDifficulty::HARD ? "Hard" : "Easy") << ")\n";
    cout << "2. Debug Mode Battle\n";
    cout << "3. Gauntlet Mode (AI: Hard)\n";
    cout << "4. Character Creator\n";
    cout << "5. Set AI Difficulty\n";
    cout << "6. Exit\n\n";
    cout << "Enter your choice: ";
}

void MainMenu::displayCreatorMenu() {
    system("cls");
    cout << "==================================\n";
    cout << "=       CHARACTER CREATOR        =\n";
    cout << "==================================\n\n";
    cout << "1. Create New Character\n";
    cout << "2. View All Characters\n";
    cout << "3. Edit Custom Character\n";
    cout << "4. Delete Custom Character\n";
    cout << "5. Back to Main Menu\n\n";
    cout << "Enter your choice: ";
}

void MainMenu::runCreator() {
    bool back = false;
    while (!back) {
        displayCreatorMenu();
        int choice = getIntInput("", 1, 5);
        switch (choice) {
        case 1: createNewCharacter(); break;
        case 2: viewCharacters(); break;
        case 3: editCharacter(); break;
        case 4: deleteCharacter(); break;
        case 5: back = true; break;
        }
    }
}

void MainMenu::run() {
    while (!exitGame) {
        displayMenu();
        int choice = getIntInput("", 1, 6);
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
        case 5: {
            system("cls");
            cout << "--- Set AI Difficulty ---\n";
            cout << "1. Easy AI\n";
            cout << "2. Hard AI\n";
            int diffChoice = getIntInput("Choose difficulty for regular battles: ", 1, 2);
            game.setAIDifficulty(diffChoice == 1 ? AIDifficulty::EASY : AIDifficulty::HARD);
            cout << "AI difficulty set to " << (game.getAIDifficulty() == AIDifficulty::HARD ? "Hard" : "Easy") << ".\n";
            cout << "Press Enter to continue...";
            cin.get();
            break;
        }
        case 6:
            exitGame = true;
            cout << "\nSaving characters and exiting...\n";
            saveCharacters();
            cout << "GAME OVER!\n";
            break;
        }
    }
}