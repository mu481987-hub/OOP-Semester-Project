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
