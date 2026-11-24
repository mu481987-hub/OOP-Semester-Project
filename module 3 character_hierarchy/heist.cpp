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

Character::Character() : characterName(""), characterRole(""), nationality(""), isCulprit(false) {}

Character::Character(const string& name, const string& role, const string& nation)
    : characterName(name), characterRole(role), nationality(nation), isCulprit(false) {
}

Character::~Character() {}

string Character::getName() const { return characterName; }
string Character::getRole() const { return characterRole; }
string Character::getNationality() const { return nationality; }
bool Character::getIsCulprit() const { return isCulprit; }
void Character::setName(const string& name) { characterName = name; }
void Character::setRole(const string& role) { characterRole = role; }
void Character::setNationality(const string& nation) { nationality = nation; }
void Character::setIsCulprit(bool value) { isCulprit = value; }
int Character::getRequiredCredibility() const { return 0; }
bool Character::isSuspect() const { return false; }

Clue Character::getClue() const {
    Clue empty;
    empty.clueLetter = '?';
    empty.cluePosition = 0;
    empty.clueRow = WORD_2;
    return empty;
}

void Character::setClue(char, int, WordRow) {}

void Character::display() const {}

ostream& operator<<(ostream& out, const Character& person) {
    out << person.characterName << " (" << person.characterRole << ", " << person.nationality << ")";
    return out;
}

istream& operator>>(istream& in, Character& person) {
    in >> person.characterName >> person.characterRole >> person.nationality;
    return in;
}

Victim::Victim(const string& name, const string& role, const string& nation)
    : Character(name, role, nation), clueRowChar(WORD_2), letterClueChar('?'), boardPosChar(0) {
}
Victim::~Victim() {}

void Victim::display() const {
    cout << "[Victim] " << *this << endl;
    cout << "  \"My locker behind the bookshelf was robbed during my party. Please find the thief!\"" << endl;
}

int Victim::getRequiredCredibility() const { return 80; }

Clue Victim::getClue() const {
    Clue c;
    c.clueLetter = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow = clueRowChar;
    return c;
}

void Victim::setClue(char letter, int position, WordRow row) {
    letterClueChar = letter;
    boardPosChar = position;
    clueRowChar = row;
}

Detective::Detective(const string& name, int startCredibility)
    : Character(name, "Detective", "Malay"), credibility(startCredibility), turns(0),
    rackWord1(7), rackWord2(8), madeWord1(""), madeWord2("") {
}
Detective::~Detective() {}

void Detective::display() const {
    cout << "[Detective] " << *this << " | Credibility: " << credibility << " | Turns used: " << turns << endl;
}

int Detective::getCredibility() const { return credibility; }
int Detective::getTurns() const { return turns; }

void Detective::loseCredibility(int points) {
    credibility -= points;
    if (credibility < 0) credibility = 0;
}

void Detective::useTurn() { turns++; }

TileRack& Detective::getRack(WordRow row) {
    if (row == WORD_1) return rackWord1;
    return rackWord2;
}

const TileRack& Detective::getRack(WordRow row) const {
    if (row == WORD_1) return rackWord1;
    return rackWord2;
}

void Detective::setMadeWord(WordRow row, const string& word) {
    if (row == WORD_1) madeWord1 = word;
    else madeWord2 = word;
}

bool Detective::isWordMade(WordRow row) const {
    if (row == WORD_1) return madeWord1 != "";
    return madeWord2 != "";
}

string Detective::getMadeWord(WordRow row) const {
    if (row == WORD_1) return madeWord1;
    return madeWord2;
}

Suspect::Suspect() : Character(), clueRowChar(WORD_2), letterClueChar('?'), boardPosChar(0) {}
Suspect::Suspect(const string& name, const string& role, const string& nation)
    : Character(name, role, nation), clueRowChar(WORD_2), letterClueChar('?'), boardPosChar(0) {
}
Suspect::~Suspect() {}

bool Suspect::isSuspect() const { return true; }

Clue Suspect::getClue() const {
    Clue c;
    c.clueLetter = letterClueChar;
    c.cluePosition = boardPosChar;
    c.clueRow = clueRowChar;
    return c;
}

void Suspect::setClue(char letter, int position, WordRow row) {
    letterClueChar = letter;
    boardPosChar = position;
    clueRowChar = row;
}

BusinessRelations::BusinessRelations(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation) {
}
BusinessRelations::~BusinessRelations() {}
int BusinessRelations::getRequiredCredibility() const { return 50; }

CloseRelations::CloseRelations(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation) {
}
CloseRelations::~CloseRelations() {}
int CloseRelations::getRequiredCredibility() const { return 75; }

HouseHelp::HouseHelp(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation) {
}
HouseHelp::~HouseHelp() {}
int HouseHelp::getRequiredCredibility() const { return 0; }

Partner::Partner(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation), BusinessRelations(name, role, nation) {
}
Partner::~Partner() {}

void Partner::display() const {
    cout << "[Business Partner] " << *this << endl;
    cout << "  \"We were arguing about the new deal in the dining room. I never touched the locker!\"" << endl;
}

PA::PA(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation), BusinessRelations(name, role, nation), CloseRelations(name, role, nation) {
}
PA::~PA() {}

void PA::display() const {
    cout << "[Personal Assistant] " << *this << endl;
    cout << "  \"I am the owner's right hand and I know his secrets. That is why I hired you.\"" << endl;
}

int PA::getRequiredCredibility() const { return 35; }

Cook::Cook(const string& name, const string& role, const string& nation)
    : HouseHelp(name, role, nation) {
}
Cook::~Cook() {}

void Cook::display() const {
    cout << "[House Help] " << *this << endl;
    cout << "  \"I was busy cooking in the kitchen all evening.\"" << endl;
}

Guard::Guard(const string& name, const string& role, const string& nation)
    : HouseHelp(name, role, nation) {
}
Guard::~Guard() {}

void Guard::display() const {
    cout << "[House Help] " << *this << endl;
    cout << "  \"I watched the garden gate the whole night.\"" << endl;
}

Servant::Servant(const string& name, const string& role, const string& nation)
    : HouseHelp(name, role, nation) {
}
Servant::~Servant() {}

void Servant::display() const {
    cout << "[House Help] " << *this << endl;
    cout << "  \"I was serving drinks and cleaning the rooms.\"" << endl;
}

OwnersChild::OwnersChild(const string& name, const string& role, const string& nation)
    : Suspect(name, role, nation), CloseRelations(name, role, nation), age(16) {
}
OwnersChild::~OwnersChild() {}

void OwnersChild::display() const {
    cout << "[Owner's Child] " << *this << ", age " << age << endl;
    cout << "  \"I was so scared! I hid upstairs when I heard the noise...\"" << endl;
}

int OwnersChild::getAge() const { return age; }

void OwnersChild::setAge(int newAge) {
    if (newAge >= 15 && newAge <= 19) age = newAge;
}

Cottage::Cottage() : locationList() {}
Cottage::Cottage(const Cottage& other) : locationList(other.locationList) {}

Cottage& Cottage::operator=(const Cottage& other) {
    if (this != &other) locationList = other.locationList;
    return *this;
}

Cottage::~Cottage() {}

void Cottage::addLocation(const Location& loc) { locationList.add(loc); }
int Cottage::getLocationCount() const { return locationList.size(); }
bool Cottage::isSearched(int index) const { return locationList[index].locSearchFlag; }
string Cottage::getLocationName(int index) const { return locationList[index].locName; }

Clue Cottage::getClue(int index) const {
    Clue c;
    c.clueLetter = locationList[index].letterClueLoc;
    c.cluePosition = locationList[index].boardPosLoc;
    c.clueRow = locationList[index].locRow;
    return c;
}

Clue Cottage::searchLocation(int index) {
    locationList[index].locSearchFlag = true;
    return getClue(index);
}

void Cottage::displayLocations() const {
    cout << endl << "====== Location Menu ======" << endl;
    for (int i = 0; i < locationList.size(); i++) {
        cout << "[" << (i + 1) << "] " << prettyName(locationList[i].locName) << " - "
            << (locationList[i].locSearchFlag ? "Searched" : "NOT Searched") << endl;
    }
    cout << "[0] Back" << endl << "==========================" << endl;
}

Scrabble::Scrabble() : row1(), row2() {}
Scrabble::Scrabble(const Scrabble& other) : row1(other.row1), row2(other.row2) {}

Scrabble& Scrabble::operator=(const Scrabble& other) {
    if (this != &other) {
        row1 = other.row1;
        row2 = other.row2;
    }
    return *this;
}

Scrabble::~Scrabble() {}

DynamicArray<Tile>& Scrabble::getRow(WordRow row) {
    if (row == WORD_1) return row1;
    return row2;
}

const DynamicArray<Tile>& Scrabble::getRow(WordRow row) const {
    if (row == WORD_1) return row1;
    return row2;
}

void Scrabble::loadWords(const string& word1, const string& word2) {
    if ((int)word1.length() != WORD1_LENGTH || (int)word2.length() != WORD2_LENGTH)
        throw FileException("clue.txt words must have 7 letters (Word 1) and 8 letters (Word 2).");
    row1.resize(WORD1_LENGTH);
    row2.resize(WORD2_LENGTH);
    for (int i = 0; i < WORD1_LENGTH; i++) {
        row1[i].tileLetter = word1[i];
        row1[i].filled = false;
    }
    for (int j = 0; j < WORD2_LENGTH; j++) {
        row2[j].tileLetter = word2[j];
        row2[j].filled = false;
    }
}

bool Scrabble::matchesClue(const Clue& clue) const {
    const DynamicArray<Tile>& tiles = getRow(clue.clueRow);
    if (clue.cluePosition < 1 || clue.cluePosition > tiles.size()) return false;
    Tile probe;
    probe.tileLetter = clue.clueLetter;
    probe.filled = false;
    return tiles[clue.cluePosition - 1] == probe;
}

void Scrabble::placeLetter(const Clue& clue) {
    DynamicArray<Tile>& tiles = getRow(clue.clueRow);
    if (tiles.size() == 0) throw InvalidWordException("No words are loaded on the board.");
    if (clue.cluePosition < 1 || clue.cluePosition > tiles.size())
        throw InvalidPositionException("Position " + numberToText(clue.cluePosition) + " is not on this word.");
    if (tiles[clue.cluePosition - 1].filled)
        throw SlotFilledException("Position " + numberToText(clue.cluePosition) + " already holds a letter.");
    if (!matchesClue(clue))
        throw InvalidLetterException(string("Letter ") + clue.clueLetter + " does not belong at position " + numberToText(clue.cluePosition) + ".");
    tiles[clue.cluePosition - 1].filled = true;
}

bool Scrabble::isRowComplete(WordRow row) const {
    const DynamicArray<Tile>& tiles = getRow(row);
    if (tiles.size() == 0) return false;
    for (int i = 0; i < tiles.size(); i++)
        if (!tiles[i].filled) return false;
    return true;
}

string Scrabble::getWord(WordRow row) const {
    if (!isRowComplete(row)) throw InvalidWordException("The row is not complete yet.");
    const DynamicArray<Tile>& tiles = getRow(row);
    string word = "";
    for (int i = 0; i < tiles.size(); i++) word += tiles[i].tileLetter;
    return word;
}

bool Scrabble::checkWord(WordRow row, const string& typedWord) const {
    return toUpperText(typedWord) == getWord(row);
}

void Scrabble::displayRow(WordRow row, ostream& out) const {
    const DynamicArray<Tile>& tiles = getRow(row);
    out << "  Word " << (int)row << ": ";
    for (int i = 0; i < tiles.size(); i++) out << tiles[i] << " ";
    if (isRowComplete(row)) out << " -> COMPLETE";
    out << endl;
}

ostream& operator<<(ostream& out, const Scrabble& board) {
    out << "Scrabble:" << endl;
    board.displayRow(WORD_1, out);
    board.displayRow(WORD_2, out);
    return out;
}

istream& operator>>(istream& in, Scrabble& board) {
    int rowNumber = 0;
    Clue clue;
    in >> rowNumber >> clue.cluePosition >> clue.clueLetter;
    clue.clueRow = (rowNumber == 1) ? WORD_1 : WORD_2;
    board.placeLetter(clue);
    return in;
}
