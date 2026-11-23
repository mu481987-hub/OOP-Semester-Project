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
    friend ostream& operator<<(ostream& out, const Tile& tile);
    friend istream& operator>>(istream& in, Tile& tile);
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
    friend ostream& operator<<(ostream& out, const Location& loc);
    friend istream& operator>>(istream& in, Location& loc);
};

// Equality operators: compare letters coming from clue.txt
bool operator==(const Tile& first, const Tile& second);
bool operator==(const Clue& first, const Clue& second);

// ------------------------------------------------------------
//  HELPER FUNCTIONS (text handling and console input)
// ------------------------------------------------------------
string trimText(const string& text);
string toUpperText(const string& text);
string prettyName(const string& text);                 // Bedroom_1 -> Bedroom 1
string numberToText(int number);
bool parseInteger(const string& text, int& value);
int splitLine(const string& line, char delimiter, string parts[], int maxParts);

// ------------------------------------------------------------
//  EXCEPTIONS (Module 7) - every message starts with HEIST ERROR
// ------------------------------------------------------------
class GameException {
protected:
    string message;
    int penalty;                    // credibility lost because of this error (0 = none)
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

// ------------------------------------------------------------
//  TEMPLATE (Module 8) - generic growable array used by
//  Cottage, Scrabble and TileRack
// ------------------------------------------------------------
template <class T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

    // Makes the internal array bigger when more room is needed.
    void reserve(int newCapacity) {
        if (newCapacity <= capacity) return;
        T* bigger = new T[newCapacity]();
        for (int i = 0; i < count; i++) bigger[i] = data[i];
        delete[] data;
        data = bigger;
        capacity = newCapacity;
    }

public:
    DynamicArray() : data(new T[4]()), count(0), capacity(4) {}

    // Deep copy constructor: the new object gets its own memory.
    DynamicArray(const DynamicArray& other)
        : data(new T[other.capacity]()), count(other.count), capacity(other.capacity) {
        for (int i = 0; i < count; i++) data[i] = other.data[i];
    }

    ~DynamicArray() { delete[] data; }

    // Deep copy assignment with self-assignment protection.
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* fresh = new T[other.capacity]();
            for (int i = 0; i < other.count; i++) fresh[i] = other.data[i];
            delete[] data;
            data = fresh;
            count = other.count;
            capacity = other.capacity;
        }
        return *this;
    }

    void add(const T& item) {
        if (count == capacity) reserve(capacity * 2);
        data[count] = item;
        count++;
    }

    // Changes the number of used items; new items get a default value.
    void resize(int newCount) {
        if (newCount < 0) newCount = 0;
        reserve(newCount);
        for (int i = count; i < newCount; i++) data[i] = T();
        count = newCount;
    }

    int size() const { return count; }

    T& operator[](int index) {
        if (index < 0 || index >= count) throw InvalidPositionException("Array index is out of range.");
        return data[index];
    }
    const T& operator[](int index) const {
        if (index < 0 || index >= count) throw InvalidPositionException("Array index is out of range.");
        return data[index];
    }
};

// ------------------------------------------------------------
//  TILE RACK (Module 5) - stores collected letters of one word.
//  Slot number (index + 1) is the board position of the letter.
// ------------------------------------------------------------
class TileRack {
private:
    DynamicArray<Tile> slots;
    int capacity;
    int count;
public:
    TileRack(int maxTiles = 8);
    TileRack(const TileRack& other);
    TileRack& operator=(const TileRack& other);
    ~TileRack();

    void addTile(char letter, int position);
    char removeTile(int position);
    void display() const;
    string toClueText() const;
    int getCount() const;
    int getCapacity() const;
    bool isFull() const;
    bool isEmpty() const;

    TileRack operator+(const TileRack& other) const;   // merge two racks with a space between
};

#endif
