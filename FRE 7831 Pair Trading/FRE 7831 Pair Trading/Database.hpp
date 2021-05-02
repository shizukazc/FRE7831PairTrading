//
//  Database.hpp
//  FRE 7831 Pair Trading
//
//  Created by Zetian Chen on 4/29/21.
//

#ifndef Database_hpp
#define Database_hpp

#include <sqlite3.h>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include "PairTrading.hpp"

#define PairTradingDBPath "./PairTrading.db"

int CreateDatabase();
int OpenDatabase(sqlite3 * &db);
int CreateTables(sqlite3 * &db);
void CloseDatabase(sqlite3 * &db);
int InsertIndividualPrices(sqlite3 * &db, std::map<std::string,Stock> &StockMap, std::vector<std::pair<std::string,std::string>> &PairMap);
//int InsertPairPrices(sqlite3 * &db, std::vector<StockPairPrices> &StockPairPricesVec);

/* Special Purpose */
int InsertPairPrices(sqlite3 * &db);
int UpdateStockPairsVolatility(sqlite3 * &db, std::string bt_startdate);

#endif /* Database_hpp */
