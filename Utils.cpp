#include "Utils.h"
#include <iostream>
#include <string>
#include <limits> 

int getIntInput(const std::string& prompt, int minVal, int maxVal) {
    int value;
    std::cout << prompt;
    while (!(std::cin >> value) || value < minVal || value > maxVal) {
        std::cout << "Invalid input. Please enter a number between " << minVal << " and " << maxVal << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Consume trailing newline
    return value;
}

std::string getStringInput(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    // Basic validation: remove leading/trailing whitespace and check for empty or semicolon
    value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
    value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

    // Loop while input is empty or contains a semicolon
    while (value.empty() || value.find(';') != std::string::npos) {
        if (value.empty()) {
            std::cout << "Input cannot be empty. ";
        }
        else if (value.find(';') != std::string::npos) {
            std::cout << "Input cannot contain semicolons (;). ";
        }
        std::cout << "Please try again: " << prompt;
        std::getline(std::cin, value);
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
    }
    return value;
}