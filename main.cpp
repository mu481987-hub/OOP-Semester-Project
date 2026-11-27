// ============================================================
//  main.cpp - Commit 02_Module07_Exception_Handling
// ============================================================
#include "heist.h"

int main() {
    try {
        throw InvalidWordException("Sample exception test", 5);
    }
    catch (const GameException& e) {
        cout << e.getMessage() << " (Penalty: " << e.getPenalty() << ")\n";
    }
    return 0;
}
