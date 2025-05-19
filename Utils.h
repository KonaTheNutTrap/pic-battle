#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <limits>   
#include <iostream> 

using namespace std;

// Function to get integer input safely
int getIntInput(const string& prompt, int minVal = numeric_limits<int>::min(), int maxVal = numeric_limits<int>::max());

// Function to get string input safely
string getStringInput(const string& prompt);

#endif // UTILS_H
