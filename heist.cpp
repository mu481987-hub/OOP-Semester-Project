// ============================================================
//  heist.cpp  -  THE LOCKER HEIST
// ============================================================
#define _CRT_SECURE_NO_WARNINGS
#include "heist.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cctype>

string trimText(const string& text) {
    int start = 0;
    int end = (int)text.length() - 1;
    while (start <= end && (text[start] == ' ' || text[start] == '\t' || text[start] == '\r' || text[start] == '\n')) start++;
    while (end >= start && (text[end] == ' ' || text[end] == '\t' || text[end] == '\r' || text[end] == '\n')) end--;
    if (start > end) return "";
    return text.substr(start, end - start + 1);
}

string toUpperText(const string& text) {
    string result = text;
    for (int i = 0; i < (int)result.length(); i++)
        result[i] = (char)toupper((unsigned char)result[i]);
    return result;
}

string prettyName(const string& text) {
    string result = text;
    for (int i = 0; i < (int)result.length(); i++)
        if (result[i] == '_') result[i] = ' ';
    return result;
}

string numberToText(int number) {
    if (number == 0) return "0";
    bool negative = number < 0;
    if (negative) number = -number;
    string digits = "";
    while (number > 0) {
        digits = (char)('0' + number % 10) + digits;
        number = number / 10;
    }
    if (negative) digits = "-" + digits;
    return digits;
}

bool parseInteger(const string& text, int& value) {
    int length = (int)text.length();
    if (length == 0 || length > 9) return false;
    int start = 0;
    bool negative = false;
    if (text[0] == '-') {
        negative = true;
        start = 1;
        if (length == 1) return false;
    }
    int result = 0;
    for (int i = start; i < length; i++) {
        if (text[i] < '0' || text[i] > '9') return false;
        result = result * 10 + (text[i] - '0');
    }
    value = negative ? -result : result;
    return true;
}

int splitLine(const string& line, char delimiter, string parts[], int maxParts) {
    int count = 0;
    string current = "";
    for (int i = 0; i < (int)line.length(); i++) {
        if (line[i] == delimiter) {
            if (count < maxParts) parts[count] = trimText(current);
            count++;
            current = "";
        } else {
            current += line[i];
        }
    }
    if (count < maxParts) parts[count] = trimText(current);
    count++;
    return count;
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    string dummy;
    if (!getline(cin, dummy)) exit(0);
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Exiting game.\n";
        exit(0);
    }
    return trimText(line);
}

int readNumber(const string& prompt) {
    string text = readLine(prompt);
    int value = 0;
    if (!parseInteger(text, value) || value < 0)
        throw InvalidInputException("Please enter a valid number.");
    return value;
}

bool askYesNo(const string& prompt) {
    while (true) {
        string answer = toUpperText(readLine(prompt));
        if (answer == "Y" || answer == "YES") return true;
        if (answer == "N" || answer == "NO") return false;
        cout << "Please type Y or N.\n";
    }
}

void printBanner(const string& title) {
    const int width = 56;
    string line = "+" + string(width, '=') + "+";
    int length = (int)title.length();
    if (length > width) length = width;
    int left = (width - length) / 2;
    int right = width - length - left;
    cout << "\n" << line << "\n";
    cout << "|" << string(left, ' ') << title << string(right, ' ') << "|\n";
    cout << line << "\n";
}

// ============================================================
//  EXCEPTIONS (Module 7)
// ============================================================
GameException::GameException(const string& text, int penaltyPoints) {
    message = "HEIST ERROR: " + text;
    penalty = penaltyPoints;
}
GameException::~GameException() {}

string GameException::getMessage() const { return message; }
int GameException::getPenalty() const { return penalty; }

InvalidWordException::InvalidWordException(const string& text, int p) : GameException("Invalid word - " + text, p) {}
RackFullException::RackFullException(const string& text, int p) : GameException("Rack full - " + text, p) {}
RackEmptyException::RackEmptyException(const string& text, int p) : GameException("Rack empty - " + text, p) {}
InvalidPositionException::InvalidPositionException(const string& text, int p) : GameException("Invalid position - " + text, p) {}
SlotFilledException::SlotFilledException(const string& text, int p) : GameException("Slot already filled - " + text, p) {}
InvalidLetterException::InvalidLetterException(const string& text, int p) : GameException("Wrong letter - " + text, p) {}
InvalidInputException::InvalidInputException(const string& text, int p) : GameException("Invalid input - " + text, p) {}
LockedCharacterException::LockedCharacterException(const string& text, int p) : GameException("Interview blocked - " + text, p) {}
FileException::FileException(const string& text, int p) : GameException("File problem - " + text, p) {}
