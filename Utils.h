#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <limits>
#include <iostream>

// Function to get integer input safely
int getIntInput(const std::string& prompt, int minVal = std::numeric_limits<int>::min(), int maxVal = std::numeric_limits<int>::max());

// Function to get string input safely
std::string getStringInput(const std::string& prompt);

#endif // UTILS_H