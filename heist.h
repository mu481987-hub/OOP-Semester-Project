// ============================================================
//  heist.h  -  THE LOCKER HEIST
//  CL1004 - Object Oriented Programming, Fall 2026
// ============================================================
#ifndef HEIST_H
#define HEIST_H

#include <iostream>
#include <string>
using namespace std;

enum WordRow { WORD_1 = 1, WORD_2 = 2 };
enum GameState { STATE_MENU, STATE_PLAYING, STATE_WON, STATE_LOST, STATE_EXIT };
enum MenuChoice { MENU_NEW_GAME = 1, MENU_LEADERBOARD, MENU_ABOUT, MENU_RULES, MENU_EXIT };

string trimText(const string& text);
string toUpperText(const string& text);
string prettyName(const string& text);
string numberToText(int number);
bool parseInteger(const string& text, int& value);
int splitLine(const string& line, char delimiter, string parts[], int maxParts);
void clearScreen();
void pauseScreen();
string readLine(const string& prompt);
int readNumber(const string& prompt);
bool askYesNo(const string& prompt);
void printBanner(const string& title);

// ------------------------------------------------------------
//  EXCEPTIONS (Module 7) - every message starts with HEIST ERROR
// ------------------------------------------------------------
class GameException {
protected:
    string message;
    int penalty;
public:
    GameException(const string& text, int penaltyPoints = 0);
    virtual ~GameException();
    virtual string getMessage() const;
    int getPenalty() const;
};

class InvalidWordException : public GameException {
public:
    InvalidWordException(const string& text, int penaltyPoints = 0);
};
class RackFullException : public GameException {
public:
    RackFullException(const string& text, int penaltyPoints = 0);
};
class RackEmptyException : public GameException {
public:
    RackEmptyException(const string& text, int penaltyPoints = 0);
};
class InvalidPositionException : public GameException {
public:
    InvalidPositionException(const string& text, int penaltyPoints = 0);
};
class SlotFilledException : public GameException {
public:
    SlotFilledException(const string& text, int penaltyPoints = 0);
};
class InvalidLetterException : public GameException {
public:
    InvalidLetterException(const string& text, int penaltyPoints = 0);
};
class InvalidInputException : public GameException {
public:
    InvalidInputException(const string& text, int penaltyPoints = 0);
};
class LockedCharacterException : public GameException {
public:
    LockedCharacterException(const string& text, int penaltyPoints = 0);
};
class FileException : public GameException {
public:
    FileException(const string& text, int penaltyPoints = 0);
};

#endif
