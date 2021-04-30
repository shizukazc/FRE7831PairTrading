//
//  Database.hpp
//  FRE 7831 Pair Trading
//
//  Created by DevonC on 4/29/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <sqlite3.h>

#define PairTradingDBPath "./PairTrading.db"

int CreateDatabase();
int OpenDatabase(sqlite3 * &db);
int CreateTables(sqlite3 * &db);
void CloseDatabase(sqlite3 * &db);

#endif /* Database_hpp */
