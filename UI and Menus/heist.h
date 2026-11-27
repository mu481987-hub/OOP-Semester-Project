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

// ------------------------------------------------------------
//  CHARACTER HIERARCHY (Modules 3 and 9)
// ------------------------------------------------------------
class GameEngine;   // forward declaration for the friend class

class Character {                         // abstract base class
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

    virtual void display() const = 0;                       // pure virtual -> abstract class
    virtual int getRequiredCredibility() const;             // credibility needed for interview
    virtual bool isSuspect() const;
    virtual Clue getClue() const;
    virtual void setClue(char letter, int position, WordRow row);

    friend class GameEngine;                                // GameEngine may use private data
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

class Suspect : public Character {        // still abstract (display is not overridden here)
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

// Virtual inheritance: BusinessRelations and CloseRelations share ONE Suspect inside PA.
class BusinessRelations : virtual public Suspect {
public:
    BusinessRelations(const string& name, const string& role, const string& nation);
    virtual ~BusinessRelations();
    int getRequiredCredibility() const override;            // 50
};

class CloseRelations : virtual public Suspect {
public:
    CloseRelations(const string& name, const string& role, const string& nation);
    virtual ~CloseRelations();
    int getRequiredCredibility() const override;            // 75
};

class HouseHelp : public Suspect {
public:
    HouseHelp(const string& name, const string& role, const string& nation);
    virtual ~HouseHelp();
    int getRequiredCredibility() const override;            // 0 = any time
};

class Partner : public BusinessRelations {
public:
    Partner(const string& name, const string& role, const string& nation);
    ~Partner();
    void display() const override;
};

class PA : public BusinessRelations, public CloseRelations {   // the diamond
public:
    PA(const string& name, const string& role, const string& nation);
    ~PA();
    void display() const override;
    int getRequiredCredibility() const override;            // 35 (resolves the ambiguity)
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
    int age;                                                // 15 to 19
public:
    OwnersChild(const string& name, const string& role, const string& nation);
    ~OwnersChild();
    void display() const override;
    int getAge() const;
    void setAge(int newAge);
};

// ------------------------------------------------------------
//  COTTAGE (Module 4) - aggregation of Location items
// ------------------------------------------------------------
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
    Clue searchLocation(int index);                         // marks as searched, returns clue
    void displayLocations() const;
};

// ------------------------------------------------------------
//  SCRABBLE BOARD (Module 4) - two rows of Tile
// ------------------------------------------------------------
class Scrabble {
private:
    DynamicArray<Tile> row1;                                // 7 slots (Word 1)
    DynamicArray<Tile> row2;                                // 8 slots (Word 2)
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

// ------------------------------------------------------------
//  LEADER / SCORE BOARD (Module 6)
// ------------------------------------------------------------
class ScoreBoard {
private:
    string filePath;
public:
    ScoreBoard(const string& path = "leader/scoreBoard.txt");
    ~ScoreBoard();
    void addScore(const string& detectiveName, const string& result, int credibility,
                  int turnsLeft, const string& culprit);
    void displayAll() const;
    static string getTodayDate();
};

// ------------------------------------------------------------
//  GAME ENGINE (Module 12) - Singleton
// ------------------------------------------------------------
class GameEngine {
private:
    static const int SUSPECT_COUNT = 7;
    static const int PEOPLE_COUNT = 8;
    static const int DEFAULT_MAX_TURNS = 25;
    static const int DEFAULT_CREDIBILITY = 100;
    static const int DEFAULT_WRONG_STEP = 5;
    static const int DEFAULT_WRONG_WORD = 10;

    // Composition: the engine owns these objects
    Detective detective;
    Cottage cottage;
    Scrabble scrabble;
    TileRack combinedRack;

    // Association: the engine only uses the score board
    ScoreBoard* scoreBoard;

    // The eight people (victim + seven suspects) are owned on the heap
    Character* people[PEOPLE_COUNT];
    bool interviewed[PEOPLE_COUNT];
    int peopleCount;

    GameState state;
    bool titleShown;

    // Rules (from rules.txt or defaults)
    int maxTurns;
    int startCredibility;
    int wrongStepPenalty;
    int wrongWordPenalty;
    string winConditionText;
    string loseConditionText;

    // Data from clue.txt and the chosen culprit
    string clueRoles[SUSPECT_COUNT];
    string clueWord1[SUSPECT_COUNT];
    string clueWord2[SUSPECT_COUNT];
    int culpritIndex;
    string culpritRole;

    // menus
    void showMainMenu();
    void handleMainMenuChoice();
    void showAbout();
    void showRules();
    void showLeaderBoard();
    void showTitle();

    // setup
    void startNewGame();
    string askDetectiveName();
    bool isValidDetectiveName(const string& name, string& reason) const;
    void loadRules();
    void loadClueFile();
    void loadCharacters();
    void loadLocations();
    void validateClueData();
    Character* createCharacter(const string& name, const string& role, const string& nation);
    void freeCharacters();
    Character* findPersonByRole(const string& role) const;

    // playing
    void playOneTurn();
    void showStatusPanel() const;
    void showGameMenu() const;
    void visitLocation();
    void interviewPerson();
    void viewBoard();
    void formWord();
    void accuseSuspect();
    void quitGame();
    void collectClue(const Clue& clue);
    void checkLoseConditions();
    void finishGame(bool won, const string& reasonText);
};

#endif
