#include "GauntletGame.h"
#include "CharacterManager.h" 
#include "Utils.h"            
#include <iostream>
#include <fstream>
#include <algorithm> 
#include <random>    
#include <cstdlib>   
#include <vector>    

using namespace std;

GauntletGame::GauntletGame() : winsInCurrentRun(0) {
    loadGauntletUnlocks();
}

Character* GauntletGame::cloneCharacter(const Character& prototype) {
    if (prototype.getName() == "OG") return new OG(dynamic_cast<const OG&>(prototype));
    if (prototype.getName() == "Helios") return new Helios(dynamic_cast<const Helios&>(prototype));
    if (prototype.getName() == "Duran") return new Duran(dynamic_cast<const Duran&>(prototype));
    if (prototype.getName() == "Philip") return new Philip(dynamic_cast<const Philip&>(prototype));
    if (prototype.getName() == "Razor") return new Razor(dynamic_cast<const Razor&>(prototype));
    if (prototype.getName() == "Sunny") return new Sunny(dynamic_cast<const Sunny&>(prototype));

   

    // New instances based on name for built-ins.
    // Avoids issues with copying a base slice if Character prototypes are used.
    if (prototype.getName() == "OG") return new OG();
    if (prototype.getName() == "Helios") return new Helios();
    if (prototype.getName() == "Duran") return new Duran();
    if (prototype.getName() == "Philip") return new Philip();
    if (prototype.getName() == "Razor") return new Razor();
    if (prototype.getName() == "Sunny") return new Sunny();
    return new Character(prototype); // Relies on Character's copy constructor.
}


void GauntletGame::loadGauntletUnlocks() {
    unlockedGauntletCharacters.clear();
    ifstream inFile(GAUNTLET_UNLOCKS_FILE);
    string name;
    if (inFile.is_open()) {
        while (getline(inFile, name)) {
            if (!name.empty()) {
                unlockedGauntletCharacters.push_back(name);
            }
        }
        inFile.close();
    }

    // Ensure "OG" is always unlocked
    if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), "OG") == unlockedGauntletCharacters.end()) {
        unlockedGauntletCharacters.push_back("OG");
        // If OG was missing and added, save immediately.
        // This also creates the file on first ever run if it didn't exist.
        saveGauntletUnlocks();
    }
    sort(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end());
}

void GauntletGame::saveGauntletUnlocks() {
    ofstream outFile(GAUNTLET_UNLOCKS_FILE);
    if (outFile.is_open()) {
        // Make sure OG is saved if it's in the list, which it should be.
        bool og_found = false;
        for (const string& name : unlockedGauntletCharacters) {
            outFile << name << endl;
            if (name == "OG") og_found = true;
        }
        if (!og_found && find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), "OG") != unlockedGauntletCharacters.end()) {
            // This case should ideally not happen if OG is always added to the vector.
            // But as a safeguard if the vector somehow lost OG before saving.
        }
        else if (!og_found) {
            outFile << "OG" << endl; // Ensure OG is saved if somehow removed from vector but should be there.
        }
        outFile.close();
    }
    else {
        cerr << "Error: Could not open " << GAUNTLET_UNLOCKS_FILE << " for writing!" << endl;
    }
}

bool GauntletGame::selectPlayerForGauntlet() {
    system("cls");
    cout << "=== Gauntlet Mode - Select Your Fighter ===\n";
    if (unlockedGauntletCharacters.empty()) {
        cout << "No characters unlocked for Gauntlet Mode (this shouldn't happen, OG is default).\n";
        cout << "Press Enter to return to menu...";
        cin.get();
        return false;
    }

    cout << "Available characters:\n";
    vector<Character*> selectablePlayerPrototypes;
    for (size_t i = 0; i < unlockedGauntletCharacters.size(); ++i) {
        bool found = false;
        for (const auto& masterChar : availableCharacters) {
            if (masterChar->getName() == unlockedGauntletCharacters[i]) {
                cout << (i + 1) << ". " << masterChar->getShortDescription() << endl;
                selectablePlayerPrototypes.push_back(masterChar.get());
                found = true;
                break;
            }
        }
        if (!found) {
            // This might happen if gauntlet_unlocks.txt has a name not in availableCharacters
            cout << (i + 1) << ". " << unlockedGauntletCharacters[i] << " (Error: Data not found)" << endl;
        }
    }
    if (selectablePlayerPrototypes.empty()) {
        cout << "No valid characters found for selection.\nPress Enter to return...";
        cin.get();
        return false;
    }


    int choice = getIntInput("Choose your character: ", 1, static_cast<int>(selectablePlayerPrototypes.size()));
    Character* chosenProto = selectablePlayerPrototypes[choice - 1];

    // Clone the selected character for the gauntlet run
    if (chosenProto->getName() == "OG") playerCharacter = make_unique<OG>();
    else if (chosenProto->getName() == "Helios") playerCharacter = make_unique<Helios>();
    else if (chosenProto->getName() == "Duran") playerCharacter = make_unique<Duran>();
    else if (chosenProto->getName() == "Philip") playerCharacter = make_unique<Philip>();
    else if (chosenProto->getName() == "Razor") playerCharacter = make_unique<Razor>();
    else if (chosenProto->getName() == "Sunny") playerCharacter = make_unique<Sunny>();
    else { // Fallback for potential custom characters if they were unlocked (not current scope)
        playerCharacter = make_unique<Character>(*chosenProto); // Basic copy
    }

    // Ensure currentHP is maxHP at the start of the gauntlet for the player
    playerCharacter->heal(playerCharacter->getMaxHp()); // Heal to full

    cout << "You chose: " << playerCharacter->getName() << endl;
    cout << "Press Enter to start the Gauntlet...";
    cin.get();
    return true;
}

void GauntletGame::generateOpponentOrder(vector<Character*>& currentOpponentList) {
    currentOpponentList.clear();
    vector<Character*> potentialOpponents;

    for (const auto& charPtr : availableCharacters) {
        // Opponents are characters the player *is not* currently playing as.
        // And they should ideally be built-in types for this gauntlet.
        if (charPtr->getName() != playerCharacter->getName() && charPtr->getType() == "BUILTIN") {
            potentialOpponents.push_back(charPtr.get());
        }
    }

    if (potentialOpponents.empty()) {
     
        for (const auto& charPtr : availableCharacters) {
            if (charPtr->getName() != playerCharacter->getName()) {
                potentialOpponents.push_back(charPtr.get());
            }
        }
    }

    if (potentialOpponents.empty()) {
        // Very unlikely, but if player is the only character loaded.
        // Add a default opponent or handle error.
        for (const auto& charPtr : availableCharacters) {
            if (charPtr->getName() == "OG" && playerCharacter->getName() != "OG") {
                potentialOpponents.push_back(charPtr.get());
                break;
            }
        }
        if (potentialOpponents.empty() && !availableCharacters.empty()) {
            potentialOpponents.push_back(availableCharacters[0].get()); // Last resort: first available char
        }
    }


    if (potentialOpponents.empty()) {
        cerr << "Error: No potential opponents found for the gauntlet!" << endl;
        return; // Or throw an exception
    }

    // Shuffle potential opponents
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(potentialOpponents.begin(), potentialOpponents.end(), g);

    // Select up to OPPONENTS_TO_BEAT
    for (int i = 0; i < OPPONENTS_TO_BEAT && i < potentialOpponents.size(); ++i) {
        currentOpponentList.push_back(potentialOpponents[i]);
    }
    // If fewer than 5 unique opponents, fill up by reusing (less ideal but ensures 5 fights)
    int currentSize = currentOpponentList.size();
    for (int i = 0; i < OPPONENTS_TO_BEAT - currentSize; ++i) {
        if (!potentialOpponents.empty()) { // Check if potentialOpponents is not empty
            currentOpponentList.push_back(potentialOpponents[i % potentialOpponents.size()]);
        }
        else {
            // This should not happen if the above error check for empty potentialOpponents worked.
            cerr << "Error: Ran out of opponents to fill the gauntlet." << endl;
            break;
        }
    }
}


void GauntletGame::displayBattleStatus(const Character& p1, const Character& p2) const {
    cout << "\n--- Gauntlet Battle Status --- \n";
    cout << p1.getName() << " (Player): " << p1.getCurrentHp() << "/" << p1.getMaxHp() << " HP\n";
    // Display player passives if desired
    cout << p2.getName() << " (Opponent): " << p2.getCurrentHp() << "/" << p2.getMaxHp() << " HP\n";
    cout << "-----------------------------\n\n";
}


string GauntletGame::getMoveString(int move) const {
    switch (move) {
    case 1: return "Rock";
    case 2: return "Paper";
    case 3: return "Scissors";
    default: return "Unknown";
    }
}

int GauntletGame::getRPSWinner(int playerMove, int botMove) const {
    if (playerMove == botMove) return 0; // Tie
    if ((playerMove == 1 && botMove == 3) ||
        (playerMove == 2 && botMove == 1) ||
        (playerMove == 3 && botMove == 2)) {
        return 1; // Player wins
    }
    return 2; // Bot wins
}

bool GauntletGame::runBattle(Character& activePlayer, Character& opponentProto) {
    // Opponent for this battle should be a fresh instance
    unique_ptr<Character> currentOpponent;
    // Create a new instance of the opponent based on its name/type
    if (opponentProto.getName() == "OG") currentOpponent = make_unique<OG>();
    else if (opponentProto.getName() == "Helios") currentOpponent = make_unique<Helios>();
    else if (opponentProto.getName() == "Duran") currentOpponent = make_unique<Duran>();
    else if (opponentProto.getName() == "Philip") currentOpponent = make_unique<Philip>();
    else if (opponentProto.getName() == "Razor") currentOpponent = make_unique<Razor>();
    else if (opponentProto.getName() == "Sunny") currentOpponent = make_unique<Sunny>();
    else { // Fallback for custom character prototypes (not primary for gauntlet)
        currentOpponent = make_unique<Character>(opponentProto); // Basic copy
    }
    currentOpponent->heal(currentOpponent->getMaxHp()); // Ensure opponent is full HP


    cout << "\n--- Battle Start! Player vs " << currentOpponent->getName() << " ---" << endl;

    while (!activePlayer.isDefeated() && !currentOpponent->isDefeated()) {
        system("cls");
        activePlayer.resetTurnState();
        currentOpponent->resetTurnState(); // Reset for the opponent too

        // --- Start of Turn Passives ---
        activePlayer.checkAndApplyPassives(PassiveTrigger::ON_TURN_START, activePlayer, *currentOpponent);
        if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
        currentOpponent->checkAndApplyPassives(PassiveTrigger::ON_TURN_START, *currentOpponent, activePlayer);
        if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;

        activePlayer.checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, activePlayer, *currentOpponent);
        if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
        currentOpponent->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *currentOpponent, activePlayer);
        if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;


        displayBattleStatus(activePlayer, *currentOpponent);

        cout << "Your move, " << activePlayer.getName() << ":\n";
        cout << "1. " << activePlayer.getMoveDescription(1) << "\n";
        cout << "2. " << activePlayer.getMoveDescription(2) << "\n";
        cout << "3. " << activePlayer.getMoveDescription(3) << "\n";
        int playerMove = getIntInput("Enter choice (1-3): ", 1, 3);

        int opponentMove = (rand() % 3) + 1; // Simple random move for opponent

        system("cls");
        displayBattleStatus(activePlayer, *currentOpponent); // Show status before results

        cout << activePlayer.getName() << " chose: " << getMoveString(playerMove) << "\n";
        cout << currentOpponent->getName() << " chose: " << getMoveString(opponentMove) << "\n\n";

        int rpsWinner = getRPSWinner(playerMove, opponentMove);

        if (rpsWinner == 0) { // Tie
            cout << "It's a tie!\n";
            activePlayer.checkAndApplyPassives(PassiveTrigger::ON_TIE, activePlayer, *currentOpponent);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
            currentOpponent->checkAndApplyPassives(PassiveTrigger::ON_TIE, *currentOpponent, activePlayer);
            if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;

        }
        else if (rpsWinner == 1) { // Player wins RPS
            int damage = activePlayer.calculateDamage(playerMove);
            cout << "You win the round! " << currentOpponent->getName() << " takes " << damage << " damage.\n";
            int oldOpponentHp = currentOpponent->getCurrentHp();
            currentOpponent->takeDamage(damage);

            activePlayer.checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove), activePlayer, *currentOpponent, playerMove, true);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
            activePlayer.checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, activePlayer, *currentOpponent);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;

            currentOpponent->checkAndApplyPassives(static_cast<PassiveTrigger>(opponentMove + 3), *currentOpponent, activePlayer, opponentMove, false);
            if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;
            currentOpponent->checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, *currentOpponent, activePlayer);
            if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;

            if (currentOpponent->getCurrentHp() != oldOpponentHp) {
                currentOpponent->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *currentOpponent, activePlayer);
                if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;
            }


        }
        else { // Opponent wins RPS
            int damage = currentOpponent->calculateDamage(opponentMove);
            cout << currentOpponent->getName() << " wins the round! You take " << damage << " damage.\n";
            int oldPlayerHp = activePlayer.getCurrentHp();
            activePlayer.takeDamage(damage);

            currentOpponent->checkAndApplyPassives(static_cast<PassiveTrigger>(opponentMove), *currentOpponent, activePlayer, opponentMove, true);
            if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;
            currentOpponent->checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, *currentOpponent, activePlayer);
            if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;

            activePlayer.checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove + 3), activePlayer, *currentOpponent, playerMove, false);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
            activePlayer.checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, activePlayer, *currentOpponent);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;

            if (activePlayer.getCurrentHp() != oldPlayerHp) {
                activePlayer.checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, activePlayer, *currentOpponent);
                if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
            }
        }
        if (activePlayer.isDefeated() || currentOpponent->isDefeated()) break;
        cout << "\nPress Enter for next turn...";
        cin.get();
    }
    system("cls");
    displayBattleStatus(activePlayer, *currentOpponent); // Final status

    if (activePlayer.isDefeated()) {
        cout << activePlayer.getName() << " has been defeated by " << currentOpponent->getName() << "!\n";
        return false;
    }
    else {
        cout << currentOpponent->getName() << " has been defeated!\n";
        return true;
    }
}


void GauntletGame::attemptUnlockNextCharacter() {
    // Determine the canonical order of built-in characters for unlocking
    vector<string> unlockOrder = { "OG", "Helios", "Duran", "Philip", "Razor", "Sunny" };

    string lastUnlocked = "OG"; // Default if no others are unlocked
    // Find the "latest" character in the unlockOrder that is currently unlocked
    for (int i = unlockOrder.size() - 1; i >= 0; --i) {
        if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), unlockOrder[i]) != unlockedGauntletCharacters.end()) {
            lastUnlocked = unlockOrder[i];
            break;
        }
    }

    // Find the next character in the canonical order to unlock
    bool foundLastUnlocked = false;
    string nextCharToUnlock = "";
    for (const string& charName : unlockOrder) {
        if (foundLastUnlocked) {
            // This is the character after the last unlocked one
            // Check if it's already in our unlocked list (it shouldn't be if logic is right)
            if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), charName) == unlockedGauntletCharacters.end()) {
                nextCharToUnlock = charName;
                break;
            }
        }
        if (charName == lastUnlocked) {
            foundLastUnlocked = true;
        }
    }


    if (!nextCharToUnlock.empty()) {
        // Check if it corresponds to an actual character in availableCharacters
        bool isValidChar = false;
        for (const auto& masterChar : availableCharacters) {
            if (masterChar->getName() == nextCharToUnlock && masterChar->getType() == "BUILTIN") {
                isValidChar = true;
                break;
            }
        }

        if (isValidChar) {
            unlockedGauntletCharacters.push_back(nextCharToUnlock);
            // Remove duplicates just in case, though logic should prevent it
            sort(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end());
            unlockedGauntletCharacters.erase(unique(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end()), unlockedGauntletCharacters.end());

            saveGauntletUnlocks();
            cout << "\nCongratulations! You've unlocked a new character for Gauntlet Mode: " << nextCharToUnlock << "!\n";
        }
        else if (!nextCharToUnlock.empty()) {
            cout << "\nAttempted to unlock " << nextCharToUnlock << " but it's not a valid built-in character.\n";
        }
    }
    else {
        cout << "\nAll available built-in characters have been unlocked for Gauntlet Mode!\n";
    }
}


void GauntletGame::play() {
    system("cls");
    cout << "=== Welcome to the Gauntlet! ===\n";
    cout << "Defeat " << OPPONENTS_TO_BEAT << " consecutive opponents to win.\n";
    cout << "Only OG is available initially. Win to unlock more fighters!\n";

    loadGauntletUnlocks(); // Ensure unlocks are fresh

    if (!selectPlayerForGauntlet()) {
        return; // Player selection failed or cancelled
    }

    vector<Character*> opponentOrderPrototypes;
    generateOpponentOrder(opponentOrderPrototypes);

    if (opponentOrderPrototypes.empty() || opponentOrderPrototypes.size() < OPPONENTS_TO_BEAT) {
        cout << "Not enough opponents to start the Gauntlet. (Need at least " << OPPONENTS_TO_BEAT << ")\n";
        cout << "Press Enter to return to menu...";
        cin.get();
        return;
    }

    winsInCurrentRun = 0;
    bool playerVictoriousInGauntlet = true;

    for (int i = 0; i < OPPONENTS_TO_BEAT; ++i) {
        cout << "\n--- Gauntlet: Round " << (i + 1) << " of " << OPPONENTS_TO_BEAT << " ---" << endl;
        Character* opponentProto = opponentOrderPrototypes[i]; // Get the prototype for the current opponent

        if (!runBattle(*playerCharacter, *opponentProto)) {
            playerVictoriousInGauntlet = false;
            cout << "\nYour Gauntlet run ends here.\n";
            break;
        }
        winsInCurrentRun++;
        cout << "\nVictory in round " << (i + 1) << "! Your HP: " << playerCharacter->getCurrentHp() << "/" << playerCharacter->getMaxHp() << "\n";
        if (i < OPPONENTS_TO_BEAT - 1) {
            
            int interRoundHeal = playerCharacter->getMaxHp() / 50; // e.g. 50% max HP
            playerCharacter->heal(interRoundHeal);
            cout << "You recovered " << interRoundHeal << " HP between rounds.\n";
            cout << "Press Enter for the next opponent...";
            cin.get();
        }
    }

    if (playerVictoriousInGauntlet) {
        cout << "\n****************************************\n";
        cout << "* CONGRATULATIONS! You beat the Gauntlet! *\n";
        cout << "****************************************\n";
        attemptUnlockNextCharacter();
    }
    else {
        cout << "\nBetter luck next time!\n";
    }

    cout << "You defeated " << winsInCurrentRun << " opponents.\n";
    cout << "Press Enter to return to the main menu...";
    cin.get();
}