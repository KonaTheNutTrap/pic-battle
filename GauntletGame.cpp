#include "GauntletGame.h"
#include "CharacterManager.h"
#include "Utils.h"
#include "AISystem.h" // For AI
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <cstdlib> // For system, rand, srand
#include <vector>
#include <memory> // For make_unique

using namespace std; // OK in .cpp file

GauntletGame::GauntletGame() : winsInCurrentRun(0) {
    loadGauntletUnlocks();
}

// Simplified clone logic inside selectPlayerForGauntlet and runBattle
// Character* GauntletGame::cloneCharacter(const Character& prototype) { ... }


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

    if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), "OG") == unlockedGauntletCharacters.end()) {
        unlockedGauntletCharacters.push_back("OG");
        saveGauntletUnlocks(); // Save if OG was added (also creates file if non-existent)
    }
    sort(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end());
}

void GauntletGame::saveGauntletUnlocks() {
    ofstream outFile(GAUNTLET_UNLOCKS_FILE);
    if (outFile.is_open()) {
        bool og_saved = false;
        for (const string& name : unlockedGauntletCharacters) {
            outFile << name << endl;
            if (name == "OG") og_saved = true;
        }
        // Ensure OG is saved if it was supposed to be there but somehow wasn't iterated
        if (!og_saved && find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), "OG") != unlockedGauntletCharacters.end()) {
            outFile << "OG" << endl;
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
    if (unlockedGauntletCharacters.empty()) { // Should always have OG
        cout << "No characters unlocked for Gauntlet Mode. (This shouldn't happen, OG is default).\n";
        cout << "Press Enter to return to menu...";
        // cin.ignore();
        cin.get();
        return false;
    }

    cout << "Available characters:\n";
    vector<Character*> selectablePlayerPrototypes;
    vector<string> validUnlockedNames; // To map choice back to name

    for (const string& unlockedName : unlockedGauntletCharacters) {
        bool found = false;
        for (const auto& masterChar : availableCharacters) {
            if (masterChar->getName() == unlockedName) {
                cout << (selectablePlayerPrototypes.size() + 1) << ". " << masterChar->getShortDescription() << endl;
                selectablePlayerPrototypes.push_back(masterChar.get());
                validUnlockedNames.push_back(unlockedName);
                found = true;
                break;
            }
        }
        if (!found) {
            cout << (selectablePlayerPrototypes.size() + 1) << ". " << unlockedName << " (Error: Data not found, cannot select)" << endl;
            // Don't add to selectablePlayerPrototypes or validUnlockedNames
        }
    }

    if (selectablePlayerPrototypes.empty()) {
        cout << "No valid characters found for selection.\nPress Enter to return...";
        // cin.ignore();
        cin.get();
        return false;
    }

    int choice = getIntInput("Choose your character: ", 1, static_cast<int>(selectablePlayerPrototypes.size()));
    Character* chosenProto = selectablePlayerPrototypes[choice - 1]; // This is a raw pointer to an object in availableCharacters

    // Clone the selected character for the gauntlet run
    // This logic should be robust. If new built-ins are added, they need to be here.
    if (chosenProto->getName() == "OG") playerCharacter = make_unique<OG>();
    else if (chosenProto->getName() == "Helios") playerCharacter = make_unique<Helios>();
    else if (chosenProto->getName() == "Duran") playerCharacter = make_unique<Duran>();
    else if (chosenProto->getName() == "Philip") playerCharacter = make_unique<Philip>();
    else if (chosenProto->getName() == "Razor") playerCharacter = make_unique<Razor>();
    else if (chosenProto->getName() == "Sunny") playerCharacter = make_unique<Sunny>();
    else { // For custom characters or other non-hardcoded built-ins
        // This relies on Character's copy constructor.
        // If chosenProto is a derived type not handled above, it will be sliced to Character.
        // This is acceptable if custom characters are only of base Character type functionality.
        playerCharacter = make_unique<Character>(*chosenProto);
    }

    playerCharacter->resetStatsForNewBattle(); // Full HP and reset bonus damage

    cout << "You chose: " << playerCharacter->getName() << endl;
    cout << "Press Enter to start the Gauntlet...";
    // cin.ignore();
    cin.get();
    return true;
}

void GauntletGame::generateOpponentOrder(vector<Character*>& currentOpponentList) {
    currentOpponentList.clear();
    vector<Character*> potentialOpponents;

    for (const auto& charPtr : availableCharacters) {
        if (playerCharacter && charPtr->getName() != playerCharacter->getName() && charPtr->getType() == "BUILTIN") {
            potentialOpponents.push_back(charPtr.get());
        }
    }
    // If not enough BUILTIN, consider adding CUSTOM (or allow repeats of BUILTIN)
    if (potentialOpponents.size() < OPPONENTS_TO_BEAT) {
        for (const auto& charPtr : availableCharacters) {
            if (playerCharacter && charPtr->getName() != playerCharacter->getName() && charPtr->getType() == "CUSTOM") {
                // Check if already added to avoid duplicates if a char is somehow both built-in and custom (should not happen)
                if (find(potentialOpponents.begin(), potentialOpponents.end(), charPtr.get()) == potentialOpponents.end()) {
                    potentialOpponents.push_back(charPtr.get());
                }
            }
        }
    }


    if (potentialOpponents.empty()) {
        // Fallback: if player is the only character or only one other type, use OG or first available non-player
        for (const auto& charPtr : availableCharacters) {
            if (playerCharacter && charPtr->getName() != playerCharacter->getName()) {
                potentialOpponents.push_back(charPtr.get());
                if (!potentialOpponents.empty()) break; // Take the first different one
            }
        }
        if (potentialOpponents.empty() && !availableCharacters.empty()) { // If still empty, player is the only char
            potentialOpponents.push_back(availableCharacters[0].get()); // Fight self (last resort)
            cout << "Warning: Not enough distinct opponents. You might fight yourself or clones." << endl;
        }
    }

    if (potentialOpponents.empty()) {
        cerr << "Error: No potential opponents found for the gauntlet! This should not happen." << endl;
        return;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(potentialOpponents.begin(), potentialOpponents.end(), g);

    for (int i = 0; i < OPPONENTS_TO_BEAT; ++i) {
        if (!potentialOpponents.empty()) { // Ensure we always have someone to pick
            currentOpponentList.push_back(potentialOpponents[i % potentialOpponents.size()]);
        }
        else {
            cerr << "Error: Ran out of potential opponents during selection." << endl; break;
        }
    }
}


void GauntletGame::displayBattleStatus(const Character& p1, const Character& p2) const {
    cout << "\n--- Gauntlet Battle Status --- \n";
    cout << p1.getName() << " (Player): " << p1.getCurrentHp() << "/" << p1.getMaxHp() << " HP\n";
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
    if (playerMove == botMove) return 0;
    if ((playerMove == 1 && botMove == 3) ||
        (playerMove == 2 && botMove == 1) ||
        (playerMove == 3 && botMove == 2)) {
        return 1;
    }
    return 2;
}

bool GauntletGame::runBattle(Character& activePlayer, Character& opponentProto) {
    unique_ptr<Character> currentOpponent;
    // Clone opponentProto to currentOpponent
    if (opponentProto.getName() == "OG") currentOpponent = make_unique<OG>();
    else if (opponentProto.getName() == "Helios") currentOpponent = make_unique<Helios>();
    else if (opponentProto.getName() == "Duran") currentOpponent = make_unique<Duran>();
    else if (opponentProto.getName() == "Philip") currentOpponent = make_unique<Philip>();
    else if (opponentProto.getName() == "Razor") currentOpponent = make_unique<Razor>();
    else if (opponentProto.getName() == "Sunny") currentOpponent = make_unique<Sunny>();
    else {
        currentOpponent = make_unique<Character>(opponentProto); // Copy constructor for customs
    }
    currentOpponent->resetStatsForNewBattle(); // Full HP

    cout << "\n--- Battle Start! Player vs " << currentOpponent->getName() << " ---" << endl;
    AIDifficulty gauntletAIDifficulty = AIDifficulty::HARD;

    while (!activePlayer.isDefeated() && !currentOpponent->isDefeated()) {
        system("cls");
        activePlayer.resetTurnState();
        currentOpponent->resetTurnState();

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

        cout << currentOpponent->getName() << " is thinking..." << endl;
        int opponentMove = AISystem::chooseMove(*currentOpponent, activePlayer, gauntletAIDifficulty);
        // std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Optional delay

        system("cls");
        displayBattleStatus(activePlayer, *currentOpponent);

        cout << activePlayer.getName() << " chose: " << getMoveString(playerMove) << "\n";
        cout << currentOpponent->getName() << " chose: " << getMoveString(opponentMove) << "\n\n";

        int rpsWinner = getRPSWinner(playerMove, opponentMove);

        if (rpsWinner == 0) {
            cout << "It's a tie!\n";
            activePlayer.checkAndApplyPassives(PassiveTrigger::ON_TIE, activePlayer, *currentOpponent, 0, false);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
            currentOpponent->checkAndApplyPassives(PassiveTrigger::ON_TIE, *currentOpponent, activePlayer, 0, false);
            if (currentOpponent->isDefeated() || activePlayer.isDefeated()) break;
        }
        else if (rpsWinner == 1) {
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
        else {
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
        // cin.ignore();
        cin.get();
    }
    system("cls");
    displayBattleStatus(activePlayer, *currentOpponent);

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
    vector<string> unlockOrder = { "OG", "Helios", "Duran", "Philip", "Razor", "Sunny" }; // Canonical order

    string lastUnlockedCanonical = "OG"; // Default: OG is always first
    for (int i = unlockOrder.size() - 1; i >= 0; --i) {
        // Check if this canonical character is in our list of unlocked characters
        if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), unlockOrder[i]) != unlockedGauntletCharacters.end()) {
            lastUnlockedCanonical = unlockOrder[i];
            break; // Found the latest character in the canonical order that we've unlocked
        }
    }

    string nextCharToUnlock = "";
    bool foundLast = false;
    for (const string& charInOrder : unlockOrder) {
        if (foundLast) {
            // This is the one after lastUnlockedCanonical. Is it already unlocked?
            if (find(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end(), charInOrder) == unlockedGauntletCharacters.end()) {
                nextCharToUnlock = charInOrder;
                break;
            }
        }
        if (charInOrder == lastUnlockedCanonical) {
            foundLast = true;
        }
    }


    if (!nextCharToUnlock.empty()) {
        bool isValidBuiltIn = false;
        for (const auto& masterChar : availableCharacters) {
            if (masterChar->getName() == nextCharToUnlock && masterChar->getType() == "BUILTIN") {
                isValidBuiltIn = true;
                break;
            }
        }

        if (isValidBuiltIn) {
            unlockedGauntletCharacters.push_back(nextCharToUnlock);
            sort(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end());
            unlockedGauntletCharacters.erase(unique(unlockedGauntletCharacters.begin(), unlockedGauntletCharacters.end()), unlockedGauntletCharacters.end());
            saveGauntletUnlocks();
            cout << "\nCongratulations! You've unlocked a new character for Gauntlet Mode: " << nextCharToUnlock << "!\n";
        }
        else if (!nextCharToUnlock.empty()) { // Should not happen if unlockOrder is correct
            cout << "\nTried to unlock '" << nextCharToUnlock << "' but it's not a recognized built-in character.\n";
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

    loadGauntletUnlocks();

    if (!playerCharacter) { // Ensure playerCharacter is null before selection or if a previous run failed mid-way
        playerCharacter.reset();
    }
    if (!selectPlayerForGauntlet()) {
        return;
    }
    if (!playerCharacter) { // Defensive check
        cout << "Player character selection failed unexpectedly. Returning to menu." << endl;
        // cin.ignore();
        cin.get();
        return;
    }


    vector<Character*> opponentOrderPrototypes;
    generateOpponentOrder(opponentOrderPrototypes);

    if (opponentOrderPrototypes.empty() || opponentOrderPrototypes.size() < OPPONENTS_TO_BEAT) {
        cout << "Not enough unique opponents to start the Gauntlet (Need at least " << OPPONENTS_TO_BEAT << " distinct types potentially).\n";
        cout << "Current available opponents for order: " << opponentOrderPrototypes.size() << endl;
        cout << "Press Enter to return to menu...";
        // cin.ignore();
        cin.get();
        return;
    }

    winsInCurrentRun = 0;
    bool playerVictoriousInGauntlet = true;

    for (int i = 0; i < OPPONENTS_TO_BEAT; ++i) {
        cout << "\n--- Gauntlet: Round " << (i + 1) << " of " << OPPONENTS_TO_BEAT << " ---" << endl;
        Character* opponentProto = opponentOrderPrototypes[i];

        if (!runBattle(*playerCharacter, *opponentProto)) {
            playerVictoriousInGauntlet = false;
            cout << "\nYour Gauntlet run ends here.\n";
            break;
        }
        winsInCurrentRun++;
        cout << "\nVictory in round " << (i + 1) << "! Your HP: " << playerCharacter->getCurrentHp() << "/" << playerCharacter->getMaxHp() << "\n";
        if (i < OPPONENTS_TO_BEAT - 1) {
            int interRoundHeal = playerCharacter->getMaxHp() / 2;
            playerCharacter->heal(interRoundHeal);
            cout << "You recovered " << interRoundHeal << " HP between rounds. Current HP: " << playerCharacter->getCurrentHp() << "/" << playerCharacter->getMaxHp() << "\n";
            cout << "Press Enter for the next opponent...";
            // cin.ignore();
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
    // playerCharacter.reset(); // Reset for next gauntlet run. Done at start of selectPlayerForGauntlet
    cout << "Press Enter to return to the main menu...";
    // cin.ignore();
    cin.get();
}