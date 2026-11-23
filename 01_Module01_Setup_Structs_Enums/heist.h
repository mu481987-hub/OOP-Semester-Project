// ============================================================
//  heist.h  -  THE LOCKER HEIST  (declarations only)
//  CL1004 - Object Oriented Programming, Fall 2026
// ============================================================
#ifndef HEIST_H
#define HEIST_H

#include <iostream>
#include <string>
using namespace std;

// ------------------------------------------------------------
//  ENUMERATIONS
//  Enums give readable names to fixed choices instead of magic numbers.
// ------------------------------------------------------------
enum WordRow { WORD_1 = 1, WORD_2 = 2 };                       // which row of the Scrabble board
enum GameState { STATE_MENU, STATE_PLAYING, STATE_WON, STATE_LOST, STATE_EXIT };
enum MenuChoice { MENU_NEW_GAME = 1, MENU_LEADERBOARD, MENU_ABOUT, MENU_RULES, MENU_EXIT };

// ------------------------------------------------------------
//  STRUCTURES
//  Structs group related data together, which improves code reusability.
// ------------------------------------------------------------
struct Tile {                       // one slot of the Scrabble board or of a rack
    char tileLetter;
    bool filled;
};

struct Clue {                       // carrier that moves data from Location/Character to Scrabble
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
};

struct Location {                   // one room of the cottage (gives a letter for Word 1)
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;
};

// ------------------------------------------------------------
//  HELPER FUNCTIONS (text handling and console input)
// ------------------------------------------------------------
string trimText(const string& text);
string toUpperText(const string& text);
string prettyName(const string& text);                 // Bedroom_1 -> Bedroom 1
string numberToText(int number);
bool parseInteger(const string& text, int& value);
int splitLine(const string& line, char delimiter, string parts[], int maxParts);

#endif
