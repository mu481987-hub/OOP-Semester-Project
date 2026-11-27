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

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseScreen() {
    cout << endl << "Press Enter to continue...";
    string dummy;
    if (!getline(cin, dummy)) exit(0);
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << endl << "Input closed. Exiting game." << endl;
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
        cout << "Please type Y or N." << endl;
    }
}

void printBanner(const string& title) {
    const int width = 56;
    string line = "+" + string(width, '=') + "+";
    int length = (int)title.length();
    if (length > width) length = width;
    int left = (width - length) / 2;
    int right = width - length - left;
    cout << endl << line << endl;
    cout << "|" << string(left, ' ') << title << string(right, ' ') << "|" << endl;
    cout << line << endl;
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

ScoreBoard::ScoreBoard(const string& path) : filePath(path) {}
ScoreBoard::~ScoreBoard() {}

string ScoreBoard::getTodayDate() {
    time_t now = time(0);
    tm* parts = localtime(&now);
    string day = numberToText(parts->tm_mday);
    string month = numberToText(parts->tm_mon + 1);
    string year = numberToText((parts->tm_year + 1900) % 100);
    if (day.length() < 2) day = "0" + day;
    if (month.length() < 2) month = "0" + month;
    if (year.length() < 2) year = "0" + year;
    return day + "-" + month + "-" + year;
}

void ScoreBoard::addScore(const string& detectiveName, const string& result, int credibility,
    int turnsLeft, const string& culprit) {
    bool fileExists = false;
    {
        ifstream check(filePath.c_str());
        fileExists = check.good();
    }
    ofstream out(filePath.c_str(), ios::app);
    if (!out) throw FileException("Cannot open " + filePath + ". Make sure the 'leader' folder exists.");
    if (!fileExists) out << "Detective|Result|Credibility|Turns_Left|Culprit|Date" << endl;
    string turnsText = numberToText(turnsLeft);
    if (turnsText.length() < 2) turnsText = "0" + turnsText;
    out << detectiveName << "|" << result << "|" << credibility << "|" << turnsText << "|"
        << culprit << "|" << getTodayDate() << endl;
}

void ScoreBoard::displayAll() const {
    ifstream file(filePath.c_str());
    if (!file) throw FileException(filePath + " was not found.");
    string line;
    if (!getline(file, line)) throw FileException(filePath + " is empty or corrupted.");
    string parts[7];
    if (splitLine(trimText(line), '|', parts, 7) != 6 || parts[0] != "Detective")
        throw FileException(filePath + " has a corrupted header line.");

    cout << endl << "Detective            Result  Cred  TurnsLeft  Culprit      Date" << endl;
    cout << string(70, '-') << endl;
    int rows = 0;
    while (getline(file, line)) {
        line = trimText(line);
        if (line == "") continue;
        if (splitLine(line, '|', parts, 7) != 6) throw FileException(filePath + " has a corrupted score line.");
        string name = parts[0], result = parts[1], cred = parts[2], left = parts[3], culprit = parts[4];
        while (name.length() < 21) name += " ";
        while (result.length() < 8) result += " ";
        while (cred.length() < 6) cred += " ";
        while (left.length() < 11) left += " ";
        while (culprit.length() < 13) culprit += " ";
        cout << name << result << cred << left << culprit << parts[5] << endl;
        rows++;
    }
    if (rows == 0) cout << "(no games played yet)" << endl;
}

void GameEngine::showAbout() {
    ifstream file("about.txt");
    if (!file) throw FileException("about.txt was not found.");
    clearScreen();
    string line;
    while (getline(file, line)) cout << line << endl;
    pauseScreen();
}

void GameEngine::showRules() {
    loadRules();
    printBanner("RULES");
    cout << " Maximum turns          : " << maxTurns << endl;
    cout << " Starting credibility   : " << startCredibility << endl;
    cout << " Every wrong step       : -" << wrongStepPenalty << " credibility" << endl;
    cout << " Every wrong word       : -" << wrongWordPenalty << " credibility" << endl;
    cout << " Win condition          : " << winConditionText << endl;
    cout << " Lose condition         : " << loseConditionText << endl;
    cout << " Viewing the board is free and does not use a turn." << endl;
    pauseScreen();
}

bool GameEngine::isValidDetectiveName(const string& name, string& reason) const {
    if (name == "") { reason = "The name cannot be empty."; return false; }
    if (name.length() > 20) { reason = "The name can have at most 20 characters."; return false; }
    for (int i = 0; i < (int)name.length(); i++) {
        char c = name[i];
        if (c == ' ') { reason = "Spaces are not allowed. Use underscore ( _ ) or dot ( . ) instead of a space."; return false; }
        bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.';
        if (!allowed) { reason = "Only letters, digits, underscore ( _ ) and dot ( . ) are allowed."; return false; }
    }
    return true;
}

string GameEngine::askDetectiveName() {
    while (true) {
        string name = readLine("Enter detective name (use _ or . for space): ");
        string reason;
        if (isValidDetectiveName(name, reason)) return name;
        cout << reason << endl;
    }
}

void GameEngine::loadRules() {
    int newMax = DEFAULT_MAX_TURNS, newCred = DEFAULT_CREDIBILITY;
    int newStep = DEFAULT_WRONG_STEP, newWord = DEFAULT_WRONG_WORD;
    string newWin = "Accuse Correct Culprit", newLose = "Credibility Zero Or Turns Exhausted";
    bool good = true;

    ifstream file("rules.txt");
    if (!file) {
        good = false;
    }
    else {
        int found = 0;
        string line;
        while (good && getline(file, line)) {
            line = trimText(line);
            if (line == "") continue;
            string parts[3];
            if (splitLine(line, '=', parts, 3) != 2) { good = false; break; }
            string key = parts[0], value = parts[1];
            int number = 0;
            if (key == "Max_Turns") {
                if (!parseInteger(value, number) || number <= 0) good = false; else newMax = number;
                found++;
            }
            else if (key == "Starting_Credibility") {
                if (!parseInteger(value, number) || number <= 0) good = false; else newCred = number;
                found++;
            }
            else if (key == "Every_Wrong_Turn") {
                if (!parseInteger(value, number)) good = false; else newStep = (number < 0) ? -number : number;
                found++;
            }
            else if (key == "Every_Wrong_Word") {
                if (!parseInteger(value, number)) good = false; else newWord = (number < 0) ? -number : number;
                found++;
            }
            else if (key == "Win_Condition") {
                newWin = prettyName(value);
                found++;
            }
            else if (key == "Lose_Condition") {
                newLose = prettyName(value);
                found++;
            }
            else {
                good = false;
            }
        }
        if (found != 6) good = false;
    }

    if (good) {
        maxTurns = newMax; startCredibility = newCred;
        wrongStepPenalty = newStep; wrongWordPenalty = newWord;
        winConditionText = newWin; loseConditionText = newLose;
    }
    else {
        maxTurns = DEFAULT_MAX_TURNS; startCredibility = DEFAULT_CREDIBILITY;
        wrongStepPenalty = DEFAULT_WRONG_STEP; wrongWordPenalty = DEFAULT_WRONG_WORD;
        winConditionText = "Accuse Correct Culprit";
        loseConditionText = "Credibility Zero Or Turns Exhausted";
        cout << endl << "[Notice] rules.txt is missing or corrupt. Default rules are used." << endl;
    }
}

void GameEngine::loadClueFile() {
    ifstream file("clue.txt");
    if (!file) throw FileException("clue.txt was not found.");
    string line;
    if (!getline(file, line)) throw FileException("clue.txt is empty.");
    int count = 0;
    while (getline(file, line)) {
        line = trimText(line);
        if (line == "") continue;
        string parts[4];
        if (splitLine(line, '|', parts, 4) != 3) throw FileException("clue.txt is corrupted (each line needs 3 fields).");
        if (count >= SUSPECT_COUNT) throw FileException("clue.txt has more than 7 profiles.");
        if (parts[1].length() != 7 || parts[2].length() != 8)
            throw FileException("clue.txt is corrupted (Word_1 needs 7 letters, Word_2 needs 8).");
        clueRoles[count] = parts[0];
        clueWord1[count] = toUpperText(parts[1]);
        clueWord2[count] = toUpperText(parts[2]);
        count++;
    }
    if (count != SUSPECT_COUNT) throw FileException("clue.txt must contain exactly 7 profiles.");
}

Character* GameEngine::createCharacter(const string& name, const string& role, const string& nation) {
    if (role == "Victim") return new Victim(name, role, nation);
    if (role == "Cook") return new Cook(name, role, nation);
    if (role == "Guard") return new Guard(name, role, nation);
    if (role == "Servant") return new Servant(name, role, nation);
    if (role == "Child") return new OwnersChild(name, role, nation);
    if (role == "Partner_1" || role == "Partner_2") return new Partner(name, role, nation);
    if (role == "PA") return new PA(name, role, nation);
    throw FileException("characters.txt has an unknown role: " + role);
}

void GameEngine::freeCharacters() {
    for (int i = 0; i < peopleCount; i++) {
        delete people[i];
        people[i] = 0;
    }
    peopleCount = 0;
}

Character* GameEngine::findPersonByRole(const string& role) const {
    for (int i = 0; i < peopleCount; i++)
        if (people[i]->characterRole == role) return people[i];
    return 0;
}

void GameEngine::loadCharacters() {
    ifstream file("characters.txt");
    if (!file) throw FileException("characters.txt was not found.");
    string line;
    if (!getline(file, line)) throw FileException("characters.txt is empty.");
    while (getline(file, line)) {
        line = trimText(line);
        if (line == "") continue;
        string parts[13];
        if (splitLine(line, '|', parts, 13) != 12) throw FileException("characters.txt is corrupted (each line needs 12 fields).");
        if (peopleCount >= PEOPLE_COUNT) throw FileException("characters.txt has more than 8 people.");
        int position = 0;
        string letterText = parts[4 + culpritIndex];
        if (!parseInteger(parts[3], position) || letterText.length() != 1 || !isalpha((unsigned char)letterText[0]))
            throw FileException("characters.txt is corrupted (bad position or letter).");
        people[peopleCount] = createCharacter(parts[0], parts[1], parts[2]);
        peopleCount++;
        people[peopleCount - 1]->setClue((char)toupper((unsigned char)letterText[0]), position, WORD_2);
    }
    if (peopleCount != PEOPLE_COUNT) throw FileException("characters.txt must contain exactly 8 people.");
}

void GameEngine::loadLocations() {
    ifstream file("locations.txt");
    if (!file) throw FileException("locations.txt was not found.");
    string line;
    if (!getline(file, line)) throw FileException("locations.txt is empty.");
    int count = 0;
    while (getline(file, line)) {
        line = trimText(line);
        if (line == "") continue;
        string parts[10];
        if (splitLine(line, '|', parts, 10) != 9) throw FileException("locations.txt is corrupted (each line needs 9 fields).");
        int position = 0;
        string letterText = parts[2 + culpritIndex];
        if (!parseInteger(parts[1], position) || letterText.length() != 1 || !isalpha((unsigned char)letterText[0]))
            throw FileException("locations.txt is corrupted (bad position or letter).");
        Location loc;
        loc.locName = parts[0];
        loc.letterClueLoc = (char)toupper((unsigned char)letterText[0]);
        loc.boardPosLoc = position;
        loc.locRow = WORD_1;
        loc.locSearchFlag = false;
        cottage.addLocation(loc);
        count++;
    }
    if (count != SUSPECT_COUNT) throw FileException("locations.txt must contain exactly 7 locations.");
}

void GameEngine::validateClueData() {
    bool used1[8] = { false, false, false, false, false, false, false, false };
    bool used2[9] = { false, false, false, false, false, false, false, false, false };
    for (int i = 0; i < cottage.getLocationCount(); i++) {
        Clue c = cottage.getClue(i);
        if (!scrabble.matchesClue(c) || used1[c.cluePosition])
            throw FileException("locations.txt letters do not spell the Word 1 of clue.txt.");
        used1[c.cluePosition] = true;
    }
    for (int j = 0; j < peopleCount; j++) {
        Clue c = people[j]->getClue();
        if (!scrabble.matchesClue(c) || used2[c.cluePosition])
            throw FileException("characters.txt letters do not spell the Word 2 of clue.txt.");
        used2[c.cluePosition] = true;
    }
}

void GameEngine::startNewGame() {
    clearScreen();
    printBanner("THE LOCKER HEIST - NEW GAME");
    string name = askDetectiveName();

    showRules();
    loadClueFile();

    culpritIndex = rand() % SUSPECT_COUNT;
    culpritRole = clueRoles[culpritIndex];

    freeCharacters();
    loadCharacters();
    cottage = Cottage();
    loadLocations();
    scrabble = Scrabble();
    scrabble.loadWords(clueWord1[culpritIndex], clueWord2[culpritIndex]);
    validateClueData();

    Character* culprit = findPersonByRole(culpritRole);
    if (culprit == 0) throw FileException("clue.txt role " + culpritRole + " does not exist in characters.txt.");
    culprit->setIsCulprit(true);

    detective = Detective(name, startCredibility);
    combinedRack = TileRack(1);
    for (int i = 0; i < PEOPLE_COUNT; i++) interviewed[i] = false;

    printBanner("THE LOCKER HEIST - NEW GAME");
    cout << " Detective " << name << " has been hired." << endl;
    cout << " A culprit has been secretly chosen. The game begins!" << endl;
    cout << scrabble;
    state = STATE_PLAYING;
    pauseScreen();
}

void GameEngine::collectClue(const Clue& clue) {
    scrabble.placeLetter(clue);
    detective.getRack(clue.clueRow).addTile(clue.clueLetter, clue.cluePosition);
    cout << "Letter " << clue.clueLetter << " collected at position " << clue.cluePosition
        << " of Word " << (int)clue.clueRow << "." << endl;
    cout << "Letter has been placed." << endl;
}

void GameEngine::formWord() {
    cout << endl << "Which word do you want to form?" << endl;
    cout << "[1] Word 1" << endl << "[2] Word 2" << endl << "[0] Back" << endl;
    int pick = readNumber("Pick: ");
    if (pick == 0) return;
    if (pick != 1 && pick != 2) throw InvalidInputException("Choose 1 or 2.");
    WordRow row = (pick == 1) ? WORD_1 : WORD_2;
    if (detective.isWordMade(row)) {
        cout << "You already solved Word " << pick << ": " << detective.getMadeWord(row) << endl;
        return;
    }
    if (!scrabble.isRowComplete(row))
        throw InvalidWordException("Word " + numberToText(pick) + " is not complete yet. Collect all its letters first.", wrongStepPenalty);

    detective.useTurn();
    while (true) {
        string typed = readLine("Type the word (or 0 to stop): ");
        if (typed == "0") return;
        if (scrabble.checkWord(row, typed)) {
            detective.setMadeWord(row, toUpperText(typed));
            cout << "Correct! Word " << pick << " is " << toUpperText(typed) << "." << endl;
            break;
        }
        detective.loseCredibility(wrongWordPenalty);
        cout << "Wrong word! -" << wrongWordPenalty << " credibility. Credibility left: " << detective.getCredibility() << endl;
        if (detective.getCredibility() <= 0) return;
    }

    if (detective.isWordMade(WORD_1) && detective.isWordMade(WORD_2)) {
        Character* culprit = findPersonByRole(culpritRole);
        cout << endl << "Both words solved:" << endl << "  " << detective.getMadeWord(WORD_1) << " + " << detective.getMadeWord(WORD_2);
        if (culprit != 0) cout << " = " << culprit->getNationality() << " -> " << prettyName(culpritRole) << " is the culprit!" << endl;
        else cout << endl;
    }
}

void GameEngine::accuseSuspect() {
    int indexes[PEOPLE_COUNT];
    int total = 0;
    cout << endl << "====== Accuse a Suspect ======" << endl;
    for (int i = 0; i < peopleCount; i++) {
        if (people[i]->isSuspect()) {
            indexes[total] = i;
            total++;
            cout << "[" << total << "] " << people[i]->getName() << " (" << prettyName(people[i]->getRole()) << ")" << endl;
        }
    }
    cout << "[0] Back" << endl << "==============================" << endl;
    int pick = readNumber("Accuse: ");
    if (pick == 0) return;
    if (pick < 1 || pick > total) throw InvalidInputException("That suspect does not exist.");
    Character* accused = people[indexes[pick - 1]];

    detective.useTurn();
    cout << "Accuse: " << accused->getName() << " (" << prettyName(accused->getRole()) << ")" << endl;
    if (accused->getIsCulprit()) {
        cout << accused->getName() << " is the culprit!" << endl;
        finishGame(true, "You caught the culprit.");
    }
    else {
        detective.loseCredibility(wrongStepPenalty);
        cout << accused->getName() << " is NOT the culprit! -" << wrongStepPenalty
            << " credibility. Credibility left: " << detective.getCredibility() << endl;
        pauseScreen();
    }
}

void GameEngine::quitGame() {
    if (askYesNo("Quit to the main menu? This game will be lost (Y/N): ")) {
        cout << "Game abandoned." << endl;
        state = STATE_MENU;
        pauseScreen();
    }
}

void GameEngine::checkLoseConditions() {
    if (detective.getCredibility() <= 0) finishGame(false, "Your credibility ran out.");
    else if (detective.getTurns() >= maxTurns) finishGame(false, "You ran out of turns.");
}

void GameEngine::finishGame(bool won, const string& reasonText) {
    state = won ? STATE_WON : STATE_LOST;
    Character* culprit = findPersonByRole(culpritRole);
    string culpritText = prettyName(culpritRole);
    if (culprit != 0) culpritText += " " + culprit->getName();
    int turnsLeft = maxTurns - detective.getTurns();
    if (turnsLeft < 0) turnsLeft = 0;

    printBanner(detective.getName() + (won ? " WIN!" : " LOSE!"));
    cout << " " << reasonText << endl;
    cout << " Final credibility: " << detective.getCredibility() << " | Turns used: " << detective.getTurns()
        << " | Culprit: " << culpritText << endl;
    cout << string(58, '=') << endl;

    state = STATE_MENU;
    if (scoreBoard != 0)
        scoreBoard->addScore(detective.getName(), won ? "WIN" : "LOSE", detective.getCredibility(), turnsLeft, culpritRole);
    cout << "Score saved." << endl;

    pauseScreen();
}
