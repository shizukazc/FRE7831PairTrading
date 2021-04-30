//
//  main.cpp
//  FRE 7831 Pair Trading
//
//  Created by DevonC on 4/29/21.
//

#include <iostream>
#include "Database.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    int ret = CreateDatabase();
    
    if (ret != SQLITE_OK)
    {
        cerr << "Failed to create database " << PairTradingDBPath << endl;
        return -1;
    }
    
    cout << "Created database " << PairTradingDBPath << endl;
    
    
    sqlite3 *db;
    
    if (OpenDatabase(db) != 0) { return -1; }
    
    if (CreateTables(db) != 0) { return -1; }
    
    CloseDatabase(db);
    
    return 0;
}
