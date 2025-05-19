#include "CharacterManager.h"
#include "Utils.h"          
#include "PassiveSystem.h"  // For Passive struct and enums
#include "Character.h"      // For Character class and derived classes like OG, Helios, etc.
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>        
#include <limits>          

using namespace std;

// Definition of global available characters list
vector<unique_ptr<Character>> availableCharacters;

// Definition of save file constant
const string SAVE_FILE = "characters.txt";

void loadCharacters() {
    availableCharacters.clear();

    // Add built-in characters first
    availableCharacters.push_back(make_unique<OG>());
    availableCharacters.push_back(make_unique<Helios>());
    availableCharacters.push_back(make_unique<Duran>());
    availableCharacters.push_back(make_unique<Philip>());
    availableCharacters.push_back(make_unique<Razor>());
    availableCharacters.push_back(make_unique<Sunny>());

    ifstream infile(SAVE_FILE);
    string line;

    if (!infile) {
        cout << "No custom character file found (" << SAVE_FILE << "). Starting with built-in characters.\n";
        ofstream outfile(SAVE_FILE); // Optionally create an empty file
        outfile.close();
        return;
    }

    while (getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        stringstream ss(line);
        string segment;
        vector<string> parts;

        while (getline(ss, segment, ';')) {
            parts.push_back(segment);
        }

        if (parts.size() >= 6 && parts[0] == "CUSTOM") {
            try {
                string name = parts[1];
                int hp = stoi(parts[2]);
                int rock = stoi(parts[3]);
                int paper = stoi(parts[4]);
                int scissors = stoi(parts[5]);
                vector<Passive> passives;

                for (size_t i = 6; i < parts.size(); ++i) {
                    if (!parts[i].empty()) {
                        passives.push_back(Passive::fromString(parts[i]));
                    }
                }
                availableCharacters.push_back(make_unique<Character>(name, hp, rock, paper, scissors, passives, "CUSTOM"));
                cout << "Loaded custom character: " << name << endl;
            }
            catch (const std::invalid_argument& e) {
                cerr << "Error parsing line (invalid number): " << line << " Why: " << e.what() << endl;
            }
            catch (const std::out_of_range& e) {
                cerr << "Error parsing line (number out of range): " << line << " Why: " << e.what() << endl;
            }
            catch (...) {
                cerr << "Unknown error parsing line: " << line << endl;
            }
        }
        else if (parts[0] != "BUILTIN") { // Avoid warning for built-in if they were ever saved, doe they shouldn't be.
            cerr << "Skipping malformed line or non-custom character entry: " << line << endl;
        }
    }
    cout << "Finished loading characters. Total characters: " << availableCharacters.size() << endl;
    infile.close();
}

void saveCharacters() {
    ofstream outfile(SAVE_FILE);
    if (!outfile) {
        cerr << "Error: Could not open " << SAVE_FILE << " for writing!" << endl;
        return;
    }

    outfile << "# Format: TYPE;NAME;HP;ROCK;PAPER;SCISSORS;PASSIVE1_STR;PASSIVE2_STR;..." << endl;
    outfile << "# Passive Str: TRIGGER_ID,EFFECT_ID,VALUE,THRESHOLD" << endl;

    for (const auto& characterPtr : availableCharacters) {
        if (characterPtr->getType() == "CUSTOM") {
            outfile << characterPtr->getType() << ";";
            outfile << characterPtr->getName() << ";";
            outfile << characterPtr->getMaxHp() << ";";
            outfile << characterPtr->getRockDamage() << ";";
            outfile << characterPtr->getPaperDamage() << ";";
            outfile << characterPtr->getScissorsDamage();
            for (const auto& p : characterPtr->getPassives()) {
                outfile << ";" << p.toString();
            }
            outfile << endl;
        }
    }
    outfile.close();
    cout << "Custom characters saved to " << SAVE_FILE << endl;
}

void displayPassiveOptions() {
    cout << "\n--- Passive Triggers ---\n";
    cout << static_cast<int>(PassiveTrigger::ON_WIN_ROCK) << ": On winning with Rock\n";
    cout << static_cast<int>(PassiveTrigger::ON_WIN_PAPER) << ": On winning with Paper\n";
    cout << static_cast<int>(PassiveTrigger::ON_WIN_SCISSORS) << ": On winning with Scissors\n";
    cout << static_cast<int>(PassiveTrigger::ON_LOSE_ROCK) << ": On losing to Rock\n";
    cout << static_cast<int>(PassiveTrigger::ON_LOSE_PAPER) << ": On losing to Paper\n";
    cout << static_cast<int>(PassiveTrigger::ON_LOSE_SCISSORS) << ": On losing to Scissors\n";
    cout << static_cast<int>(PassiveTrigger::ON_TIE) << ": On a tie\n";
    cout << static_cast<int>(PassiveTrigger::ON_HP_BELOW_PERCENT) << ": When HP is below a % threshold\n";
    cout << static_cast<int>(PassiveTrigger::ON_TURN_START) << ": At the start of your turn\n";
    cout << static_cast<int>(PassiveTrigger::AFTER_ANY_ATTACK) << ": After you attack (win or lose)\n";
    cout << static_cast<int>(PassiveTrigger::AFTER_TAKING_HIT) << ": After taking damage\n";


    cout << "\n--- Passive Effects ---\n";
    cout << static_cast<int>(PassiveEffect::HEAL_SELF_FLAT) << ": Heal self (flat amount)\n";
    cout << static_cast<int>(PassiveEffect::DAMAGE_OPPONENT_FLAT) << ": Damage opponent (flat amount)\n";
    cout << static_cast<int>(PassiveEffect::INCREASE_NEXT_ATTACK_FLAT) << ": Increase next attack damage (flat amount)\n";
    cout << static_cast<int>(PassiveEffect::INCREASE_ROCK_DMG_PERM) << ": Permanently increase Rock damage\n";
    cout << static_cast<int>(PassiveEffect::INCREASE_PAPER_DMG_PERM) << ": Permanently increase Paper damage\n";
    cout << static_cast<int>(PassiveEffect::INCREASE_SCISSORS_DMG_PERM) << ": Permanently increase Scissors damage\n";
    cout << static_cast<int>(PassiveEffect::HEAL_SELF_PERCENT_CURRENT) << ": Heal self (% of current HP)\n";
    cout << static_cast<int>(PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT) << ": Damage opponent (% of their current HP)\n";
    cout << "Enter " << static_cast<int>(PassiveTrigger::NONE) << " for trigger or " << static_cast<int>(PassiveEffect::NONE) << " for effect to skip adding a passive.\n";
}

void createNewCharacter() {
    system("cls");
    cout << "=== Create New Character ===\n\n";

    string name = getStringInput("Enter character name: ");

    bool nameExists = false;
    for (const auto& ch : availableCharacters) {
        if (ch->getName() == name) {
            nameExists = true;
            break;
        }
    }
    if (nameExists) {
        cout << "Error: A character with this name already exists.\n";
        cout << "Press Enter to return to the menu...";
        cin.get(); 
        return;
    }

    int hp = getIntInput("Enter Max HP (1-100): ", 1, 100);
    int rock = getIntInput("Enter Rock Damage (0-10): ", 0, 10);
    int paper = getIntInput("Enter Paper Damage (0-10): ", 0, 10);
    int scissors = getIntInput("Enter Scissors Damage (0-10): ", 0, 10);

    vector<Passive> passives;
    cout << "\n--- Add Passives (up to 3, enter 0 to skip) ---" << endl;

    for (int i = 0; i < 3; ++i) {
        cout << "\n-- Passive " << (i + 1) << " --\n";
        displayPassiveOptions();

        PassiveTrigger trigger = static_cast<PassiveTrigger>(getIntInput("Choose Trigger ID: ", 0, static_cast<int>(PassiveTrigger::AFTER_TAKING_HIT)));
        if (trigger == PassiveTrigger::NONE) {
            cout << "Skipping remaining passives.\n";
            break;
        }

        PassiveEffect effect = static_cast<PassiveEffect>(getIntInput("Choose Effect ID: ", 0, static_cast<int>(PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT)));
        if (effect == PassiveEffect::NONE) {
            cout << "Skipping this passive.\n";
            continue;
        }

        int value = 0;
        int threshold = 0;

        if (effect == PassiveEffect::HEAL_SELF_PERCENT_CURRENT || effect == PassiveEffect::DAMAGE_OPPONENT_PERCENT_CURRENT) {
            value = getIntInput("Enter Percentage Value (1-100): ", 1, 100);
        }
        else {
            value = getIntInput("Enter Flat Value (1-50): ", 1, 50);
        }

        if (trigger == PassiveTrigger::ON_HP_BELOW_PERCENT) {
            threshold = getIntInput("Enter HP Threshold Percentage (1-99): ", 1, 99);
        }

        passives.emplace_back(trigger, effect, value, threshold);
        cout << "Added Passive: " << passives.back().getDescription() << endl;

        if (i < 2) {
            char addAnother = ' ';
            cout << "Add another passive? (y/n): ";
            cin >> addAnother;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (tolower(addAnother) != 'y') {
                break;
            }
        }
    }

    availableCharacters.push_back(make_unique<Character>(name, hp, rock, paper, scissors, passives, "CUSTOM"));
    cout << "\nCharacter '" << name << "' created successfully!\n";
    saveCharacters();
    cout << "Press Enter to return to the menu...";
    cin.get();
}

void viewCharacters() {
    system("cls");
    cout << "=== Available Characters ===\n\n";
    if (availableCharacters.empty()) {
        cout << "No characters available. Load or create some first.\n";
    }
    else {
        for (size_t i = 0; i < availableCharacters.size(); ++i) {
            cout << (i + 1) << ". [" << availableCharacters[i]->getType() << "] "
                << availableCharacters[i]->getFullDescription() << "\n" << endl;
        }
    }
    cout << "Press Enter to return to the menu...";
    cin.get(); 
}

void deleteCharacter() {
    system("cls");
    cout << "=== Delete Custom Character ===\n\n";

    vector<size_t> customCharIndices;
    cout << "Select a custom character to delete:\n";
    cout << "0. Cancel\n";
    for (size_t i = 0; i < availableCharacters.size(); ++i) {
        if (availableCharacters[i]->getType() == "CUSTOM") {
            cout << (customCharIndices.size() + 1) << ". " << availableCharacters[i]->getName() << endl;
            customCharIndices.push_back(i);
        }
    }

    if (customCharIndices.empty()) {
        cout << "\nNo custom characters to delete.\n";
        cout << "Press Enter to return to the menu...";
        cin.get(); 
        return;
    }

    int choice = getIntInput("Enter choice: ", 0, static_cast<int>(customCharIndices.size()));

    if (choice == 0) {
        cout << "Deletion cancelled.\n";
    }
    else {
        size_t originalIndex = customCharIndices[choice - 1];
        string deletedName = availableCharacters[originalIndex]->getName();
        availableCharacters.erase(availableCharacters.begin() + originalIndex);
        cout << "Character '" << deletedName << "' deleted.\n";
        saveCharacters();
    }
    cout << "Press Enter to return to the menu...";
    cin.get(); 
}