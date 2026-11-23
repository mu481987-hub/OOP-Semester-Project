#define _CRT_SECURE_NO_WARNINGS
#include "heist.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cctype>

static bool isTrimChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

string trimText(const string& text) {
    int start = 0;
    int end = (int)text.length() - 1;
    while (start <= end && isTrimChar(text[start])) start++;
    while (end >= start && isTrimChar(text[end])) end--;
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
        }
        else {
            current += line[i];
        }
    }
    if (count < maxParts) parts[count] = trimText(current);
    count++;
    return count;
}

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

ostream& operator<<(ostream& out, const Tile& tile) {
    if (tile.filled) out << "[" << tile.tileLetter << "]";
    else out << "[ ]";
    return out;
}

istream& operator>>(istream& in, Tile& tile) {
    in >> tile.tileLetter;
    tile.filled = true;
    return in;
}

ostream& operator<<(ostream& out, const Location& loc) {
    out << prettyName(loc.locName) << " (letter " << loc.letterClueLoc << ", position "
        << loc.boardPosLoc << ", Word " << (int)loc.locRow << ", "
        << (loc.locSearchFlag ? "Searched" : "NOT Searched") << ")";
    return out;
}

istream& operator>>(istream& in, Location& loc) {
    in >> loc.locName >> loc.letterClueLoc >> loc.boardPosLoc;
    loc.locRow = WORD_1;
    loc.locSearchFlag = false;
    return in;
}

bool operator==(const Tile& first, const Tile& second) {
    return first.tileLetter == second.tileLetter;
}

bool operator==(const Clue& first, const Clue& second) {
    return first.clueLetter == second.clueLetter && first.cluePosition == second.cluePosition
        && first.clueRow == second.clueRow;
}

TileRack::TileRack(int maxTiles) : slots(), capacity(maxTiles), count(0) {
    if (capacity < 1) capacity = 1;
    slots.resize(capacity);
    for (int i = 0; i < capacity; i++) {
        slots[i].tileLetter = ' ';
        slots[i].filled = false;
    }
}

TileRack::TileRack(const TileRack& other) : slots(other.slots), capacity(other.capacity), count(other.count) {}

TileRack& TileRack::operator=(const TileRack& other) {
    if (this != &other) {
        slots = other.slots;
        capacity = other.capacity;
        count = other.count;
    }
    return *this;
}

TileRack::~TileRack() {}

void TileRack::addTile(char letter, int position) {
    if (isFull()) throw RackFullException("This rack cannot hold more letters.");
    if (position < 1 || position > capacity) throw InvalidPositionException("Rack position " + numberToText(position) + " does not exist.");
    if (slots[position - 1].filled) throw SlotFilledException("Rack position " + numberToText(position) + " already holds a letter.");
    slots[position - 1].tileLetter = letter;
    slots[position - 1].filled = true;
    count++;
}

char TileRack::removeTile(int position) {
    if (isEmpty()) throw RackEmptyException("There is no letter to remove.");
    if (position < 1 || position > capacity) throw InvalidPositionException("Rack position " + numberToText(position) + " does not exist.");
    if (!slots[position - 1].filled) throw InvalidPositionException("No letter is stored at position " + numberToText(position) + ".");
    char letter = slots[position - 1].tileLetter;
    slots[position - 1].filled = false;
    slots[position - 1].tileLetter = ' ';
    count--;
    return letter;
}

void TileRack::display() const {
    if (count == 0) {
        cout << "(empty)";
        return;
    }
    for (int i = 0; i < capacity; i++)
        if (slots[i].filled) cout << slots[i].tileLetter << "(" << (i + 1) << ") ";
}

string TileRack::toClueText() const {
    string text = "";
    for (int i = 0; i < capacity; i++)
        text += slots[i].filled ? slots[i].tileLetter : '_';
    return text;
}

int TileRack::getCount() const { return count; }
int TileRack::getCapacity() const { return capacity; }
bool TileRack::isFull() const { return count == capacity; }
bool TileRack::isEmpty() const { return count == 0; }

TileRack TileRack::operator+(const TileRack& other) const {
    TileRack merged(capacity + 1 + other.capacity);
    for (int i = 0; i < capacity; i++) merged.slots[i] = slots[i];
    merged.slots[capacity].tileLetter = ' ';
    merged.slots[capacity].filled = true;
    for (int j = 0; j < other.capacity; j++) merged.slots[capacity + 1 + j] = other.slots[j];
    merged.count = count + other.count;
    return merged;
}
