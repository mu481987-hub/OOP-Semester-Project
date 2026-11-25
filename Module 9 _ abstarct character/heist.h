
#ifndef HEIST_H
#define HEIST_H

#include <iostream>
#include <string>
using namespace std;

enum WordRow { WORD_1 = 1, WORD_2 = 2 };
enum GameState { STATE_MENU, STATE_PLAYING, STATE_WON, STATE_LOST, STATE_EXIT };
enum MenuChoice { MENU_NEW_GAME = 1, MENU_LEADERBOARD, MENU_ABOUT, MENU_RULES, MENU_EXIT };

struct Tile {
    char tileLetter;
    bool filled;
    friend ostream& operator<<(ostream& out, const Tile& tile);
    friend istream& operator>>(istream& in, Tile& tile);
};

struct Clue {
    char clueLetter;
    int cluePosition;
    WordRow clueRow;
};

struct Location {
    string locName;
    char letterClueLoc;
    int boardPosLoc;
    WordRow locRow;
    bool locSearchFlag;
    friend ostream& operator<<(ostream& out, const Location& loc);
    friend istream& operator>>(istream& in, Location& loc);
};

bool operator==(const Tile& first, const Tile& second);
bool operator==(const Clue& first, const Clue& second);

string trimText(const string& text);
string toUpperText(const string& text);
string prettyName(const string& text);
string numberToText(int number);
bool parseInteger(const string& text, int& value);
int splitLine(const string& line, char delimiter, string parts[], int maxParts);

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

template <class T>
class DynamicArray {
private:
    T* data;
    int count;
    int capacity;

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

    DynamicArray(const DynamicArray& other)
        : data(new T[other.capacity]()), count(other.count), capacity(other.capacity) {
        for (int i = 0; i < count; i++) data[i] = other.data[i];
    }

    ~DynamicArray() {
        delete[] data;
    }

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

    void resize(int newCount) {
        if (newCount < 0) newCount = 0;
        reserve(newCount);
        for (int i = count; i < newCount; i++) data[i] = T();
        count = newCount;
    }

    int size() const {
        return count;
    }

    T& operator[](int index) {
        if (index < 0 || index >= count)
            throw InvalidPositionException("Array index is out of range.");
        return data[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= count)
            throw InvalidPositionException("Array index is out of range.");
        return data[index];
    }
};

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

    TileRack operator+(const TileRack& other) const;
};

class GameEngine;

class Character {
private:
    string characterName;
    string characterRole;
    string nationality;
    bool isCulprit;

public:
    Character();
    Character(const string& name, const string& role, const string& nation);
    virtual ~Character();

    string getName() const;
    string getRole() const;
    string getNationality() const;
    bool getIsCulprit() const;
    void setName(const string& name);
    void setRole(const string& role);
    void setNationality(const string& nation);
    void setIsCulprit(bool value);

    virtual void display() const = 0;
    virtual int getRequiredCredibility() const;
    virtual bool isSuspect() const;
    virtual Clue getClue() const;
    virtual void setClue(char letter, int position, WordRow row);

    friend class GameEngine;
    friend ostream& operator<<(ostream& out, const Character& person);
    friend istream& operator>>(istream& in, Character& person);
};

class Victim : public Character {
private:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Victim(const string& name, const string& role, const string& nation);
    ~Victim();
    void display() const override;
    int getRequiredCredibility() const override;
    Clue getClue() const override;
    void setClue(char letter, int position, WordRow row) override;
};

class Detective : public Character {
private:
    int credibility;
    int turns;
    TileRack rackWord1;
    TileRack rackWord2;
    string madeWord1;
    string madeWord2;

public:
    Detective(const string& name = "Agent", int startCredibility = 100);
    ~Detective();
    void display() const override;
    int getCredibility() const;
    int getTurns() const;
    void loseCredibility(int points);
    void useTurn();
    TileRack& getRack(WordRow row);
    const TileRack& getRack(WordRow row) const;
    void setMadeWord(WordRow row, const string& word);
    bool isWordMade(WordRow row) const;
    string getMadeWord(WordRow row) const;
};

class Suspect : public Character {
private:
    WordRow clueRowChar;
    char letterClueChar;
    int boardPosChar;

public:
    Suspect();
    Suspect(const string& name, const string& role, const string& nation);
    virtual ~Suspect();
    bool isSuspect() const override;
    Clue getClue() const override;
    void setClue(char letter, int position, WordRow row) override;
};

class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& name, const string& role, const string& nation);
    virtual ~BusinessRelations();
    int getRequiredCredibility() const override;
};

class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& name, const string& role, const string& nation);
    virtual ~CloseRelations();
    int getRequiredCredibility() const override;
};

class HouseHelp : public Suspect {
public:
    HouseHelp(const string& name, const string& role, const string& nation);
    virtual ~HouseHelp();
    int getRequiredCredibility() const override;
};

class Partner : public BusinessRelations {
public:
    Partner(const string& name, const string& role, const string& nation);
    ~Partner();
    void display() const override;
};

class PA : public BusinessRelations, public CloseRelations {
public:
    PA(const string& name, const string& role, const string& nation);
    ~PA();
    void display() const override;
    int getRequiredCredibility() const override;
};

class Cook : public HouseHelp {
public:
    Cook(const string& name, const string& role, const string& nation);
    ~Cook();
    void display() const override;
};

class Guard : public HouseHelp {
public:
    Guard(const string& name, const string& role, const string& nation);
    ~Guard();
    void display() const override;
};

class Servant : public HouseHelp {
public:
    Servant(const string& name, const string& role, const string& nation);
    ~Servant();
    void display() const override;
};

class OwnersChild : public CloseRelations {
private:
    int age;

public:
    OwnersChild(const string& name, const string& role, const string& nation);
    ~OwnersChild();
    void display() const override;
    int getAge() const;
    void setAge(int newAge);
};

class Cottage {
private:
    DynamicArray<Location> locationList;

public:
    Cottage();
    Cottage(const Cottage& other);
    Cottage& operator=(const Cottage& other);
    ~Cottage();

    void addLocation(const Location& loc);
    int getLocationCount() const;
    bool isSearched(int index) const;
    string getLocationName(int index) const;
    Clue getClue(int index) const;
    Clue searchLocation(int index);
    void displayLocations() const;
};

class Scrabble {
private:
    DynamicArray<Tile> row1;
    DynamicArray<Tile> row2;
    static const int WORD1_LENGTH = 7;
    static const int WORD2_LENGTH = 8;

    DynamicArray<Tile>& getRow(WordRow row);
    const DynamicArray<Tile>& getRow(WordRow row) const;

public:
    Scrabble();
    Scrabble(const Scrabble& other);
    Scrabble& operator=(const Scrabble& other);
    ~Scrabble();

    void loadWords(const string& word1, const string& word2);
    bool matchesClue(const Clue& clue) const;
    void placeLetter(const Clue& clue);
    bool isRowComplete(WordRow row) const;
    bool checkWord(WordRow row, const string& typedWord) const;
    string getWord(WordRow row) const;
    void displayRow(WordRow row, ostream& out = cout) const;

    friend ostream& operator<<(ostream& out, const Scrabble& board);
    friend istream& operator>>(istream& in, Scrabble& board);
};

#endif
