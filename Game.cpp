#include "Game.h"
#include "CharacterManager.h" 
#include "Utils.h"           
#include <iostream>
#include <cstdlib>           
#include <algorithm>          // For potential future use

using namespace std;

Game::Game() : player(nullptr), bot(nullptr), debugMode(false) {}

Game::~Game() {} // Raw pointers player/bot point to objects managed by unique_ptr in availableCharacters

void Game::setDebugMode(bool debug) {
    debugMode = debug;
}

int Game::getRandomMove() const {
    return rand() % 3 + 1; // 1 = Rock, 2 = Paper, 3 = Scissors
}

void Game::displayHealth() const {
    cout << "\n===== STATUS =====\n";
    if (player) { // Ensure player is not null
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

    if (bot) { // Ensure bot is not null
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
    if (playerMove == botMove) return 0; // Tie
    if ((playerMove == 1 && botMove == 3) ||
        (playerMove == 2 && botMove == 1) ||
        (playerMove == 3 && botMove == 2)) {
        return 1; // Player wins
    }
    return 2; // Bot wins
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

    player = selectCharacter("Select your Fighter!");
    if (!player) return false;

    if (availableCharacters.size() <= 1) {
        cout << "Not enough unique characters for the bot to choose! Bot will be the same.\n";
        bot = player;
    }
    else {
        int botChoiceIndex;
        do {
            botChoiceIndex = rand() % availableCharacters.size();
        } while (availableCharacters[botChoiceIndex].get() == player);
        bot = availableCharacters[botChoiceIndex].get();
    }

    cout << "\nYou chose: " << player->getName() << "\n";
    cout << "Enemy chose: " << bot->getName() << "\n\n";
    cout << "Let the battle commence!\n";
    cout << "Press Enter to start...";
    cin.get();
    return true;
}

bool Game::initializeDebug() {
    system("cls");
    cout << "=== DEBUG MODE: PIC BATTLE ===\n\n";

    player = selectCharacter("Select Player's Fighter!");
    if (!player) return false;

    bot = selectCharacter("Select Bot's Fighter!");
    if (!bot) return false;

    cout << "\nPlayer is: " << player->getName() << "\n";
    cout << "Bot is: " << bot->getName() << "\n\n";
    cout << "Let the debug battle commence!\n";
    cout << "Press Enter to start...";
    cin.get();
    return true;
}

void Game::playRound() {
    system("cls");

    if (!player || !bot) { // Safety check
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

    int botMove;
    if (debugMode) {
        cout << "\nDEBUG MODE: Choose Bot's move:\n";
        cout << "1. " << bot->getMoveDescription(1) << "\n";
        cout << "2. " << bot->getMoveDescription(2) << "\n";
        cout << "3. " << bot->getMoveDescription(3) << "\n";
        botMove = getIntInput("Enter Bot's choice (1-3): ", 1, 3);
    }
    else {
        botMove = getRandomMove();
    }

    system("cls");
    displayHealth();

    cout << "\nYou chose: " << getMoveString(playerMove) << "\n";
    cout << "Bot chose: " << getMoveString(botMove) << "\n\n";

    int winner = getRPSWinner(playerMove, botMove);

    if (winner == 0) { // Tie
        cout << "It's a tie!\n";
        player->checkAndApplyPassives(PassiveTrigger::ON_TIE, *player, *bot);
        if (bot->isDefeated() || player->isDefeated()) return;
        bot->checkAndApplyPassives(PassiveTrigger::ON_TIE, *bot, *player);
        if (player->isDefeated() || bot->isDefeated()) return;

    }
    else if (winner == 1) { // Player wins round
        int damage = player->calculateDamage(playerMove);
        cout << "You win this round! Bot (" << bot->getName() << ") takes " << damage << " damage.\n";
        int oldBotHp = bot->getCurrentHp();
        bot->takeDamage(damage);

        player->checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove), *player, *bot, playerMove, true); // ON_WIN_MOVE
        if (bot->isDefeated() || player->isDefeated()) return;
        player->checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, *player, *bot);
        if (bot->isDefeated() || player->isDefeated()) return;

        bot->checkAndApplyPassives(static_cast<PassiveTrigger>(botMove + 3), *bot, *player, botMove, false); // ON_LOSE_MOVE
        if (player->isDefeated() || bot->isDefeated()) return;
        bot->checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, *bot, *player);
        if (player->isDefeated() || bot->isDefeated()) return;

        if (bot->getCurrentHp() != oldBotHp) { // Check HP change for ON_HP_BELOW_PERCENT
            bot->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *bot, *player);
            if (player->isDefeated() || bot->isDefeated()) return;
        }


    }
    else { // Bot wins round
        int damage = bot->calculateDamage(botMove);
        cout << "Bot wins this round! You (" << player->getName() << ") take " << damage << " damage.\n";
        int oldPlayerHp = player->getCurrentHp();
        player->takeDamage(damage);

        bot->checkAndApplyPassives(static_cast<PassiveTrigger>(botMove), *bot, *player, botMove, true); // ON_WIN_MOVE
        if (player->isDefeated() || bot->isDefeated()) return;
        bot->checkAndApplyPassives(PassiveTrigger::AFTER_ANY_ATTACK, *bot, *player);
        if (player->isDefeated() || bot->isDefeated()) return;

        player->checkAndApplyPassives(static_cast<PassiveTrigger>(playerMove + 3), *player, *bot, playerMove, false); // ON_LOSE_MOVE
        if (bot->isDefeated() || player->isDefeated()) return;
        player->checkAndApplyPassives(PassiveTrigger::AFTER_TAKING_HIT, *player, *bot);
        if (bot->isDefeated() || player->isDefeated()) return;

        if (player->getCurrentHp() != oldPlayerHp) { // Check HP change for ON_HP_BELOW_PERCENT
            player->checkAndApplyPassives(PassiveTrigger::ON_HP_BELOW_PERCENT, *player, *bot);
            if (bot->isDefeated() || player->isDefeated()) return;
        }
    }
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