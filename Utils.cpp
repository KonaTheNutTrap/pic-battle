#include "Utils.h"
#include <iostream>
#include <string>
#include <limits>

using namespace std;

int getIntInput(const string& prompt, int minVal, int maxVal) {
    int value;
    cout << prompt;
    while (!(cin >> value) || value < minVal || value > maxVal) {
        cout << "Invalid input. Please enter a number between " << minVal << " and " << maxVal << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Consume trailing newline
    return value;
}

string getStringInput(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    // Basic validation: remove leading/trailing whitespace and check for empty or semicolon
    value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
    value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
    while (value.empty() || value.find(';') != string::npos) {
        if (value.empty()) cout << "Input cannot be empty. ";
        else cout << "Input cannot contain semicolons (;). ";
        cout << "Please try again: ";
        getline(cin, value);
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
    }
    return value;
}