#include "Game.h"
#include "CharacterManager.h"
#include "Utils.h"
#include "AISystem.h"
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>

using namespace std; 

Game::Game() : player(nullptr), bot(nullptr), debugMode(false),
currentAIDifficulty(AIDifficulty::HARD), lastPlayerMove(0), lastBotMove(0) {
}

Game::~Game() {}

void Game::setDebugMode(bool debug) {
    debugMode = debug;
}

void Game::setAIDifficulty(AIDifficulty difficulty) {
    currentAIDifficulty = difficulty;
}

AIDifficulty Game::getAIDifficulty() const {
    return currentAIDifficulty;
}

void Game::resetBattleHistory() {
    lastPlayerMove = 0;
    lastBotMove = 0;
}

void Game::displayHealth() const {
    cout << "\n===== STATUS =====\n";
    if (player) {
        cout << player->getName() << ": " << player->getCurrentHp() << "/" << player->getMaxHp() << " HP\n";
        if (!player->getPassives().empty()) {
            cout << "  Passives:\n";
            for (const auto& p : player->getPassives()) {
                cout << "    - " << p.getDescription() << "\n";
            }
        }
    }
    else {
        cout << "Player not selected.\n";
    }
    cout << "\n";
    if (bot) {
        cout << "Bot (" << bot->getName() << "): " << bot->getCurrentHp() << "/" << bot->getMaxHp() << " HP\n";
        if (!bot->getPassives().empty()) {
            cout << "  Passives:\n";
            for (const auto& p : bot->getPassives()) {
                cout << "    - " << p.getDescription() << "\n";
            }
        }
    }
    else {
        cout << "Bot not selected.\n";
    }
    cout << "=================\n\n";
}

int Game::getRPSWinner(int playerMove, int botMove) const {
    if (playerMove == botMove) return 0;
    if ((playerMove == 1 && botMove == 3) ||
        (playerMove == 2 && botMove == 1) ||
        (playerMove == 3 && botMove == 2)) {
        return 1;
    }
    return 2;
}

string Game::getMoveString(int move) const {
    switch (move) {
    case 1: return "Rock";
    case 2: return "Paper";
    case 3: return "Scissors";
    default: return "Unknown";
    }
}

Character* Game::selectCharacter(const string& prompt) {
    cout << prompt << endl;
    if (availableCharacters.empty()) {
        cerr << "Error: No characters available to select!" << endl;
        return nullptr;
    }

    for (size_t i = 0; i < availableCharacters.size(); ++i) {
        cout << (i + 1) << ". " << availableCharacters[i]->getShortDescription() << endl;
    }

    int choice = getIntInput("Enter choice: ", 1, static_cast<int>(availableCharacters.size()));
    return availableCharacters[choice - 1].get();
}

bool Game::initialize() {
    system("cls");
    cout << "=== PIC BATTLE ===\n\n";
    resetBattleHistory(); // Reset history for a new battle

    player = selectCharacter("Select your Fighter!");
    if (!player) return false;

    if (availableCharacters.size() <= 1 && (availableCharacters.empty() || availableCharacters[0].get() == player)) {
        cout << "Not enough unique characters for the bot to choose! Bot will be the same.\n";
        bot = player;
        if (availableCharacters.empty()) {
            cerr << "Critical Error: No characters available for bot after player selection." << endl; return false;
        }
        else if (!bot) {
            bot = availableCharacters[0].get();
        }
    }
    else {
        vector<Character*> potentialBots;
        for (const auto& charPtr : availableCharacters) {
            if (charPtr.get() != player) {
                potentialBots.push_back(charPtr.get());
            }
        }
        if (potentialBots.empty()) {
            cout << "Only one character available. Bot will be the same as player." << endl;
            bot = player;
        }
        else {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, potentialBots.size() - 1);
            bot = potentialBots[distrib(gen)];
        }
    }
    if (!bot) {
        cerr << "Error: Bot could not be selected." << endl;
        if (!availableCharacters.empty()) bot = availableCharacters[0].get();
        else return false;
    }

    cout << "\nYou chose: " << player->getName() << "\n";
    cout << "Enemy chose: " << bot->getName() << "\n\n";
    if (player) player->resetStatsForNewBattle();
    if (bot) bot->resetStatsForNewBattle();
    cout << "Let the battle commence!\n";
    cout << "Press Enter to start...";
    cin.get();
    return true;
}

bool Game::initializeDebug() {
    system("cls");
    cout << "=== DEBUG MODE: PIC BATTLE ===\n\n";
    resetBattleHistory(); // Reset history for a new battle

    player = selectCharacter("Select Player's Fighter!");
    if (!player) return false;

    bot = selectCharacter("Select Bot's Fighter!");
    if (!bot) return false;

    cout << "\nPlayer is: " << player->getName() << "\n";
    cout << "Bot is: " << bot->getName() << "\n\n";
    if (player) player->resetStatsForNewBattle();
    if (bot) bot->resetStatsForNewBattle();
    cout << "Let the debug battle commence!\n";
    cout << "Press Enter to start...";
    cin.get();
    return true;
}

void Game::playRound() {
    system("cls");

    if (!player || !bot) {
        cout << "Error: Player or Bot not initialized for the round." << endl;
        return;
    }

    player->resetTurnState();
    bot->resetTurnState();

    player->checkAndApplyPassives(PassiveTrigger::ON_TURN_START, *player, *bot);
    if (bot->isDefeated() || player->isDefeated()) return;
    bot->checkAndApplyPassives(PassiveTrigger::ON_TURN_START, *bot, *player);
    if (player->isDefeated() || bot->isDefeated()) return;

    player->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *player, *bot);
    if (bot->isDefeated() || player->isDefeated()) return;
    bot->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *bot, *player);
    if (player->isDefeated() || bot->isDefeated()) return;

    displayHealth();

    cout << "Choose your move:\n";
    cout << "1. " << player->getMoveDescription(1) << "\n";
    cout << "2. " << player->getMoveDescription(2) << "\n";
    cout << "3. " << player->getMoveDescription(3) << "\n";
    int playerMove = getIntInput("Enter choice (1-3): ", 1, 3);

    int currentBotMove; // Renamed to avoid confusion with member lastBotMove
    if (debugMode) {
        cout << "\nDEBUG MODE: Bot is " << bot->getName() << ". Choose Bot's move (or 4 for AI):\n";
        cout << "1. " << bot->getMoveDescription(1) << "\n";
        cout << "2. " << bot->getMoveDescription(2) << "\n";
        cout << "3. " << bot->getMoveDescription(3) << "\n";
        cout << "4. Let AI (" << (currentAIDifficulty == AIDifficulty::HARD ? "Hard" : "Easy") << ") choose for Bot\n";
        int choice = getIntInput("Enter Bot's choice (1-4): ", 1, 4);
        if (choice == 4) {
            currentBotMove = AISystem::chooseMove(*bot, *player, currentAIDifficulty, lastBotMove, lastPlayerMove);
            cout << "AI for " << bot->getName() << " chose: " << getMoveString(currentBotMove) << endl;
            cout << "Press Enter to see result...";
            cin.get();
        }
        else {
            currentBotMove = choice;
        }
    }
    else {
        cout << "Bot (" << bot->getName() << ") is thinking..." << endl;
        currentBotMove = AISystem::chooseMove(*bot, *player, currentAIDifficulty, lastBotMove, lastPlayerMove);
    }

    system("cls");
    displayHealth();

    cout << "\nYou (" << player->getName() << ") chose: " << getMoveString(playerMove) << "\n";
    cout << "Bot (" << bot->getName() << ") chose: " << getMoveString(currentBotMove) << "\n\n";

    int winner = getRPSWinner(playerMove, currentBotMove);

    if (winner == 0) { // Tie
        cout << "It's a tie!\n";
        player->checkAndApplyPassives(PassiveTrigger::ON_TIE, *player, *bot, 0, false);
        if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        bot->checkAndApplyPassives(PassiveTrigger::ON_TIE, *bot, *player, 0, false);
        if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }

    }
    else if (winner == 1) { // Player wins round
        int damage = player->calculateDamage(playerMove);
        cout << "You win this round! Bot (" << bot->getName() << ") takes " << damage << " damage.\n";
        int oldBotHp = bot->getCurrentHp();
        bot->takeDamage(damage);

        player->checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove), *player, *bot, playerMove, true);
        if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        player->checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, *player, *bot);
        if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }

        bot->checkAndApplyPassives(static_cast<PassiveTrigger>(currentBotMove + 3), *bot, *player, currentBotMove, false);
        if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        bot->checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, *bot, *player);
        if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }

        if (bot->getCurrentHp() != oldBotHp) {
            bot->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *bot, *player);
            if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        }
    }
    else { // Bot wins round
        int damage = bot->calculateDamage(currentBotMove);
        cout << "Bot wins this round! You (" << player->getName() << ") take " << damage << " damage.\n";
        int oldPlayerHp = player->getCurrentHp();
        player->takeDamage(damage);

        bot->checkAndApplyPassives(static_cast<PassiveTrigger>(currentBotMove), *bot, *player, currentBotMove, true);
        if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        bot->checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, *bot, *player);
        if (player->isDefeated() || bot->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }

        player->checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove + 3), *player, *bot, playerMove, false);
        if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        player->checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, *player, *bot);
        if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }

        if (player->getCurrentHp() != oldPlayerHp) {
            player->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *player, *bot);
            if (bot->isDefeated() || player->isDefeated()) { lastPlayerMove = playerMove; lastBotMove = currentBotMove; return; }
        }
    }
    // Update history for next round's AI
    this->lastPlayerMove = playerMove;
    this->lastBotMove = currentBotMove;
}

bool Game::isGameOver() const {
    if (!player || !bot) return true;
    return player->isDefeated() || bot->isDefeated();
}

void Game::announceWinner() const {
    if (!player || !bot) {
        cout << "\nGame ended prematurely due to character selection issue.\n";
        return;
    }
    if (player->isDefeated() && bot->isDefeated()) {
        cout << "\nDOUBLE K.O.! Both fighters are defeated.\n";
    }
    else if (player->isDefeated()) {
        cout << "\nDEFEAT! Bot (" << bot->getName() << ") wins with "
            << bot->getCurrentHp() << " HP remaining.\n";
    }
    else {
        cout << "\nVICTORY! You (" << player->getName() << ") won with " << player->getCurrentHp()
            << " HP remaining. Bot (" << bot->getName() << ") is defeated.\n";
    }
}

void Game::play() {
    bool initialized = false;
    resetBattleHistory(); // Ensure history is clean at the start of a new game sequence
    if (debugMode) {
        initialized = initializeDebug();
    }
    else {
        initialized = initialize();
    }

    if (!initialized || !player || !bot) {
        cout << "Failed to initialize game. Returning to menu.\n";
        cout << "Press Enter to continue...";
        cin.get();
        return;
    }

    while (!isGameOver()) {
        playRound();
        if (isGameOver()) break;
        cout << "\nPress Enter to continue to the next round...";
        cin.get();
    }

    system("cls");
    displayHealth();
    announceWinner();

    cout << "\nBattle finished!\n";
    cout << "Press Enter to return to main menu...";
    cin.get();
}